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
#include <functional>

#include"cmdln.h"
#include"util.h"
#include"topic_model.h"
#include"vectorManager.h"
#include"labelManager.h"
#include"estimateVector.h"
#include"metrics.h"
#include"parallelFileOp.h"

using namespace std;

string label2pmid(const string& label){
  if(label.substr(0, 4) != "PMID")
    throw runtime_error("Attempted to convert non-pmid label to pmid");
  size_t underscoreIdx = label.find('_');
  return label.substr(4, underscoreIdx-4);
}


bool verbose;
#define vout if(::verbose) cout

int main(int argc, char ** argv){

  cmdline::parser p;

  p.add<string>("cloud", 'c', "Cloud file", true);
  p.add<string>("inference", 'i', "Inference file", true);
  p.add<string>("labels", 'l', "Label file", true);
  p.add<string>("output", 'o', "Path to resulting file", true);
  p.add<string>("topic-model", 'm', "Topic model from VIEW_FILES", true);
  p.add("verbose", 'v', "Output debug info.");
  p.add<size_t>("papers-per-topic", 'X', "Num. pmid per topic", false, 5);


  p.parse_check(argc, argv);

  string cloudPath = p.get<string>("cloud");
  string inferencePath = p.get<string>("inference");
  string labelPath =  p.get<string>("labels");
  string outPath = p.get<string>("output");
  string topicModelPath = p.get<string>("topic-model");
  ::verbose = p.exist("verbose");
  size_t papersPerTopic = p.get<size_t>("papers-per-topic");

  vout << "Loading Labels:";
  LabelManager labels(labelPath);
  vout << "  -- Done --" << endl;

  vout << "Loading topic model from " << topicModelPath;
  vector<Topic> topicModel = getTM(topicModelPath);
  vout << "  -- Done --" << endl;

  vout << "Loading inferences";
  unordered_map<string, vector<float>> pmid2mix;
  nodeIdx id;
  string line;
  float tmp;
  fstream cloudFile(cloudPath, ios::in);
  fstream inferenceFile(inferencePath, ios::in);
  // Note: one line of cloud file is equal to one line in inf
  while(cloudFile >> id && getline(inferenceFile, line)){
    stringstream ss(line);
    vector<float> mix;
    mix.reserve(topicModel.size());
    float sum = 0;
    while(ss >> tmp){
      mix.push_back(tmp);
      sum += tmp;
    }
    //    mix /= sum; // normalize to prob func
    assert(mix.size() == topicModel.size());
    const string& pmid = labels[id];
    pmid2mix[label2pmid(pmid)] = move(mix);
  }
  cloudFile.close();
  inferenceFile.close();
  vout << "  -- Done --" << endl;


  vout << "Determining best papers for mixtures";

  fstream outFile(outPath, ios::out);
  for(size_t topic_idx = 0; topic_idx < topicModel.size(); ++topic_idx){
    vector<pair<float, string>> score_pmid;
    score_pmid.reserve(pmid2mix.size());
    for(const auto& pmid_scores : pmid2mix){
      const string& pmid = pmid_scores.first;
      float score = pmid_scores.second[topic_idx];
      score_pmid.push_back({score, pmid});
    }
    //sort in reverse
    sort(score_pmid.begin(), score_pmid.end(), greater<pair<float, string>>());
    outFile << "Topic_" << topic_idx << " ";
    for(size_t i = 0; i < min(papersPerTopic, score_pmid.size()); ++i){
      outFile << score_pmid[i].second << " ";
      cout << score_pmid[i].second << " " << score_pmid[i].first << endl;
    }
    outFile << endl;
  }

  outFile.close();

  return 0;
}
