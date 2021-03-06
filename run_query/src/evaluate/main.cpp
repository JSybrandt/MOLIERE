/*
 * The goal here is to calculate some statistics about each of the predicate TM's that we generate.
 * This means that we need to read a predicate, a TM file, and the synonyms of that word.
 *
 *
 * How hard is it to load this file... thats worth a check
 * It takes less than half a second to load the file, we can do that.
 */

#include<iostream>
#include<unordered_map>
#include<map>
#include<unordered_set>
#include<fstream>
#include<sstream>
#include<cmath>
#include<omp.h>
#include<limits>
#include<algorithm>

#include"cmdln.h"
#include"files.h"
#include"util.h"
#include"topic_model.h"
#include"vectorManager.h"
#include"labelManager.h"
#include"estimateVector.h"

#include"metrics.h"

using namespace std;

bool verbose;
#define vout if(::verbose) cout

typedef tuple<float, float, float, float> FourFloats;

const unordered_map<string, FourFloats> DEFAULT_CONFIG {
  {"L2",                FourFloats(-0.9746f, 1.5492f, 0.0f, 13.1183f )},
  {"BestCentrL2",       FourFloats(+0.5131f, 2.6658f, 0.0f, 0.62918f )},
  {"BestTopicPerWord",  FourFloats(+0.0717f, 2.7610f, 0.0f, 0.869151f )},
  {"TopicCorr",         FourFloats(-0.2275f, 1.9070f, 0.0f, 0.999596f )},
  {"TopicWalkBtwn",     FourFloats(-0.3946f, 1.1746f, 0.0f, 3019.73f )},
  {"TopicNetCCoef",     FourFloats(-0.4087f, 2.0228f, 0.0f, 0.607236f )}
};

unordered_map<string, FourFloats> getHParam(const string& path){
  if(isFile(path)){
    vout << "Loading Hyper Param from " << path << endl;
    unordered_map<string, FourFloats> param;
    fstream fin(path, ios::in);
    string name;
    float coef, expo, minVal, range;

    // load from file
    while(fin >> name >> coef >> expo >> minVal >> range){
      if(DEFAULT_CONFIG.find(name) == DEFAULT_CONFIG.end()){
        throw runtime_error("Illegal hyperparam file");
      }
      param[name] = make_tuple(coef, expo, minVal, range);
    }
    fin.close();
    return param;
  } else {
    vout << "Using Default Param" << endl;
    return DEFAULT_CONFIG;
  }
}

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("labels", 'l', "Label file", true);
  p.add<string>("graph", 'g', "Edges file", true);
  p.add<string>("output", 'o', "Path to resulting file", true);
  p.add<string>("topic-model", 'm', "Topic model from VIEW_FILES", true);
  p.add<string>("ngram-vecs", 'N', "ngram vector file", false, "");
  p.add<string>("pmid-vecs", 'P', "pmid vector file", false, "");
  p.add<string>("umls-vecs", 'U', "cuid vector file", false, "");
  p.add<string>("source-label", 's', "Source Label", true);
  p.add<string>("target-label", 't', "Target Label", true);
  p.add("verbose", 'v', "Output debug info.");
  p.add<size_t>("reported-topics", 'r', "Number of topics to report", false, 5);
  p.add<size_t>("topic-cutoff", 'f', "number of words to be reported per topic", false, 5);
  p.add<string>("hyper-parameter-config", 'h', "file describing coef. and exp. for metric.", false, "");
  p.add<string>("topic-network", 'n', "Where to write topic nn network to.", false, "");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graph");
  string labelPath =  p.get<string>("labels");
  string outPath = p.get<string>("output");
  string topicModelPath = p.get<string>("topic-model");
  string ngramVecsPath = p.get<string>("ngram-vecs");
  string pmidVecsPath = p.get<string>("pmid-vecs");
  string umlsVecsPath = p.get<string>("umls-vecs");
  string sourceLabel = p.get<string>("source-label");
  string targetLabel = p.get<string>("target-label");
  ::verbose = p.exist("verbose");
  size_t numReportedTopics = p.get<size_t>("reported-topics");
  size_t topicCutoff = p.get<size_t>("topic-cutoff");
  string hyperParamPath = p.get<string>("hyper-parameter-config");
  string topicNetworkPath = p.get<string>("topic-network");

  unordered_map<string, FourFloats> hyperParam = getHParam(hyperParamPath);

  vout << "Loading topic model from " << topicModelPath << endl;
  vector<Topic> topicModel = getTM(topicModelPath);

  vout << "Getting all unique wordsInTopicModel" << endl;
  unordered_set<string> wordsInTopicModel;
  wordsInTopicModel.insert(sourceLabel);
  wordsInTopicModel.insert(targetLabel);
  for(const Topic & topic : topicModel){
    for(const auto & p : topic){
      wordsInTopicModel.insert(p.first);
    }
  }

  vout << "Getting Necessary Vectors" << endl;
  VectorManager vectors;
  vectors.registerTag(ngramVecsPath);
  vectors.registerTag(pmidVecsPath, 'P');
  vectors.registerTag(umlsVecsPath, 'C');
  unordered_map<string, vector<float>> word2vec = vectors.getVectors(wordsInTopicModel);

  vout << "Getting Centroids" << endl;
  vector<vector<float>> topicCentroids;
  topicCentroids.reserve(topicModel.size());
  for(const Topic& t: topicModel){
    topicCentroids.push_back(getCentroid(t, word2vec));
  }

  pair<string, string> queryWords = {sourceLabel, targetLabel};

  vout << "Checking that vectors exist. Making estimates if not" << endl;
  vector<string> needed;
  if(word2vec.find(sourceLabel) == word2vec.end()){
    vout << "MISSING: " << sourceLabel << endl;
    needed.push_back(sourceLabel);
  }
  if(word2vec.find(targetLabel) == word2vec.end()){
    vout << "MISSING: " << targetLabel << endl;
    needed.push_back(targetLabel);
  }

  if(needed.size() > 0){
    vout << "Loading labels for reconstruction." << endl;
    LabelManager labels(labelPath);
    for(string& s : needed){
      vout << "Reconstructing: " << s << endl;
      word2vec[s] = estimateVector(s, graphPath, vectors, labels);
    }
  }

  vout << "Creating network data" << endl;
  TopicNetworkMetricData netData(queryWords, topicModel, topicCentroids, word2vec);

  if(topicNetworkPath != ""){
    vout << "Writing topic network" << endl;

    const vector<nodeIdx> path = netData.getTopicPath();
    unordered_set<nodeIdx> importantTopics(path.begin(), path.end());

    // idx to human readable text, star means along path
    auto idx2name = [&sourceLabel, &targetLabel, &importantTopics](nodeIdx i) -> string{
      if(i==0) return sourceLabel;
      if(i==1) return targetLabel;
      stringstream ss;
      ss << "Topic_" << i-2;
      if(importantTopics.find(i) != importantTopics.end()){
        ss << "*";
      }
      return ss.str();
    };

    fstream topNetFile(topicNetworkPath, ios::out);
    topNetFile << "Source,Target,Weight" << endl;
    for(const edge& e : netData.getSimpleGraph().toEdgeList()){
      topNetFile << idx2name(e.a) << "," << idx2name(e.b) << "," << e.weight << endl;
    }
    topNetFile.close();
  }

  vout << "Creating Metric Map" << endl;
  // want order
  map<string, Metric*> metrics = {
    {"L2",
      new L2Metric(queryWords, topicModel, topicCentroids, word2vec)},
    {"BestCentrL2",
      new BestCentrL2Metric(queryWords, topicModel, topicCentroids, word2vec)},
    {"BestTopicPerWord",
      new BestTopicPerWordMetric(queryWords, topicModel, topicCentroids, word2vec)},
    {"TopicCorr",
      new TopicCorrMetric(queryWords, topicModel, topicCentroids, word2vec)},
    {"TopicWalkBtwn",
      new TopicWalkBtwnMetric(netData)},
    {"TopicNetCCoef",
      new TopicNetCCoefMetric(netData)},
  };

  fstream outFile(outPath, ios::out);
  float result = 0;
  for(const auto& p : hyperParam){
    const string& metric = p.first;
    float coef = get<0>(p.second);
    float expo = get<1>(p.second);
    float minVal = get<2>(p.second);
    float range = get<3>(p.second);

    float score = metrics[metric]->calculate();
    outFile << metric << "\t" << score;

    //if its a per-word metric
    PerTopicMetric* perTopic = dynamic_cast<PerTopicMetric*>(metrics[metric]);
    if(perTopic){
      vector<pair<size_t, float>> topic2score = perTopic->getBestTopics();
      for(size_t t = 0; t < min(topicModel.size(), numReportedTopics); ++t){
        size_t topicIdx = topic2score[t].first;
        // print topic number
        outFile << "\tTopic_" << topicIdx;
        for(size_t w = 0; w < min(topicCutoff, topicModel[topicIdx].size()); ++w){
          // print topic words
          outFile << " " << topicModel[topicIdx][w].first;
        }
      }
    }
    outFile << endl;
    if(score >= minVal) //dont do negative
      result += coef * pow((score-minVal)/range, expo);
  }
  outFile << "PolyMultiple\t" << result << endl;

  for(auto& p : metrics){
    delete p.second;
  }
  outFile.close();
  return 0;
}
