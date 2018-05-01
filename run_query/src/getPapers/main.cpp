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
#include"util.h"
#include"topic_model.h"
#include"vectorManager.h"
#include"labelManager.h"
#include"estimateVector.h"
#include"metrics.h"
#include"parallelFileOp.h"

using namespace std;

bool verbose;
#define vout if(::verbose) cout

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("cloud", 'c', "Cloud file", true);
  p.add<string>("inference", 'i', "Inference file", true);
  // p.add<string>("abstracts", 'a', "Abstract file", true);
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
  p.add<size_t>("papers-per-topic", 'X', "Num. pmid per topic", false, 5);


  p.parse_check(argc, argv);

  string cloudPath = p.get<string>("cloud");
  string inferencePath = p.get<string>("inference");
  // string abstractPath = p.get<string>("abstracts");
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
  size_t papersPerTopic = p.get<size_t>("papers-per-topic");

  vout << "Loading Labels:";
  LabelManager labels(labelPath);
  vout << "  -- Done --" << endl;

  vout << "Loading topic model from " << topicModelPath;
  vector<Topic> topicModel = getTM(topicModelPath);
  vout << "  -- Done --" << endl;

  vout << "Getting all unique wordsInTopicModel";
  unordered_set<string> wordsInTopicModel;
  wordsInTopicModel.insert(sourceLabel);
  wordsInTopicModel.insert(targetLabel);
  for(const Topic & topic : topicModel){
    for(const auto & p : topic){
      wordsInTopicModel.insert(p.first);
    }
  }
  vout << "  -- Done --" << endl;

  //vout << "Getting Necessary Vectors";
  //VectorManager vectors;
  //vectors.registerTag(ngramVecsPath);
  //vectors.registerTag(pmidVecsPath, 'P');
  //vectors.registerTag(umlsVecsPath, 'C');
  //unordered_map<string, vector<float>> word2vec = vectors.getVectors(wordsInTopicModel);
  //vout << "  -- Done --" << endl;

  //vout << "Getting Topic Centroids";
  //vector<vector<float>> topicCentroids;
  //topicCentroids.reserve(topicModel.size());
  //for(const Topic& t: topicModel){
    //topicCentroids.push_back(getCentroid(t, word2vec));
  //}
  //vout << "  -- Done --" << endl;

  //pair<string, string> queryWords = {sourceLabel, targetLabel};

  //vout << "Checking query word vectors exist. Making estimates if not";
  //if(word2vec.find(sourceLabel) == word2vec.end()){
    //vout << " MISSING: " << sourceLabel;
    //word2vec[sourceLabel] = estimateVector(sourceLabel, graphPath, vectors, labels);
  //}
  //if(word2vec.find(targetLabel) == word2vec.end()){
    //vout << " MISSING: " << targetLabel;
    //word2vec[targetLabel] = estimateVector(targetLabel, graphPath, vectors, labels);
  //}
  //vout << "  -- Done --" << endl;

  //vout << "Creating NN Net";
  //// NOTE: query words are idx 0 and 1, 2-n are the topics (0-(n-2))
  //vector<nodeIdx> topicPath = simpleTopicNetData(queryWords,
                                                 //topicModel,
                                                 //topicCentroids,
                                                 //word2vec).second;
  //vout << "  -- Done --" << endl;

  //vout << "Getting best topics from L2";
  //BestCentrL2Metric topicMet(queryWords, topicModel, topicCentroids, word2vec);
  //vout << "  -- Done --" << endl;

  // list of topic idx to score, sorted.
  //vector<pair<size_t, float>> bestTopics = topicMet.getBestTopics();

  vout << "Loading inferences";
  unordered_map<string, vector<float>> pmid2mix;
  nodeIdx id;
  string line;
  float tmp;
  fstream cloudFile(cloudPath, ios::in);
  fstream inferenceFile(inferencePath, ios::in);
  while(cloudFile >> id && getline(inferenceFile, line)){
    stringstream ss(line);
    vector<float> mix;
    mix.reserve(topicModel.size());
    float sum = 0;
    while(ss >> tmp){
      mix.push_back(tmp);
      sum += tmp;
    }
    mix /= sum; // normalize to prob func
    const string& pmid = labels[id];
    pmid2mix[pmid] = move(mix);
  }
  cloudFile.close();
  inferenceFile.close();
  vout << "  -- Done --" << endl;


  //vout << "Determining papers per topic";
  //vector<vector<float>> desiredMixes;
  //// the best mixes for each best topic is just that topic
  //for(size_t i = 0; i < topicModel.size(); ++i){
    //vector<float> mix(topicModel.size(), 0.0f);
    //mix[i] = 1;
    //desiredMixes.emplace_back(move(mix));
  //}

  //for(size_t i = 1; i < topicPath.size()-1; ++i){
    //for(size_t j = i+1; j < topicPath.size()-1; ++j){
      //vector<float> mix(topicModel.size(), 0.0f);
      //for(size_t x = i; x < j; ++x){
        //mix[x] = 1;
      //}
      //desiredMixes.emplace_back(move(mix));
    //}
  //}
  vout << "  -- Done --" << endl;

  vout << "Determining best papers for mixtures";
  vector<pQueue<string, float>> bestPapersPerMix(topicModel.size());

  for(auto& pmidMix : pmid2mix){
    for(size_t i = 0 ; i < topicModel.size(); ++i){
      bestPapersPerMix[i].push(pmidMix.first, -pmidMix.second[i]);
    }
  }
  vout << "  -- Done --" << endl;

  //vout << "Collecting Papers";
  //unordered_set<string> selectPmid;
  //for(pQueue<string, float> paperQ : bestPapersPerMix){
    //for(size_t i = 0; i < min(paperQ.size(), numPerMix); ++i){
      //selectPmid.insert(paperQ.pop().first);
    //}
  //}
  //vout << "  -- Done --" << endl;

  //vout << "Finding Abstract Text";
  //list<string> abstracts;
  //auto selectPmidFun = [&selectPmid](const string& line) -> bool {
    //stringstream ss(line);
    //string tmp;
    //ss >> tmp;
    //return selectPmid.find(tmp) != selectPmid.end();
  //};

  //auto get = [](const string& line) -> string { return line; };
  //fastProcFile<string>(abstractPath, abstracts, get, selectPmidFun);

  fstream outFile(outPath, ios::out);
  for(size_t i = 0; i < bestPapersPerMix.size(); ++i){
    outFile << "Topic_" << i << " ";
    for(size_t j = 0; j < papersPerTopic && bestPapersPerMix[i].size() > 0; ++j){
      outFile << bestPapersPerMix[i].pop().first << " ";
    }
    outFile << endl;
  }
  outFile.close();
  // vout << "  -- Done --" << endl;

  return 0;
}
