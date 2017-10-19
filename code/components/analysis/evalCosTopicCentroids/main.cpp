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
#include<unordered_set>
#include<fstream>
#include<sstream>
#include<cmath>
#include<omp.h>
#include<limits>
#include"cmdln.h"
#include"util.h"

using namespace std;

// an array of lists of pairs
typedef unordered_map<string, unsigned int> Topic;

bool verbose;
#define vout if(::verbose) cout

vector<Topic> getTM(string path){
  static const string TOPIC_TOKEN = "TOPIC:";

  string token;
  unsigned int numTopics = 0;
  // Get number of topics
  fstream tmFile(path, ios::in);
  while(tmFile >> token){
    if(token == TOPIC_TOKEN){
      numTopics++;
    }
  }
  tmFile.close();

  vector<Topic> result(numTopics);

  tmFile = fstream(path, ios::in);
  unsigned int currTopic;
  float occuranceRate;
  while(tmFile >> token){
    if(token == TOPIC_TOKEN){
      tmFile >> currTopic;
      tmFile >> occuranceRate;
    } else {
      tmFile >> occuranceRate;
      // NOTE: we have to load as float because that the file.
      // All the values are positive integers
      // token = processString(token);
      result[currTopic][token] = occuranceRate;
    }
  }
  tmFile.close();

  return result;
}

vector<float> getTopicCentroid(const Topic & topic, const unordered_map<string, vector<float>> & word2vec){
  vector<float> res(word2vec.begin()->second.size(), 0);
  float totalCount = 0;
  for(const auto & pair : topic){
    const string& word = pair.first;
    float occuranceCount = pair.second;
    const auto it = word2vec.find(word);
    if(it != word2vec.end()){
      res += it->second * occuranceCount;
      totalCount += pair.second;
    } else {
      vout << "Unable to process " << word << endl;
    }
  }
  res /= float(totalCount);
  return res;
}

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("topicModel", 'm', "Topic model from VIEW_FILES", true);
  p.add<string>("ngramVecs", 'n', "ngram vector file", false, "");
  p.add<string>("pmidVecs", 'p', "pmid vector file", false, "");
  p.add<string>("cuidVecs", 'c', "cuid vector file", false, "");
  p.add<string>("sourceLabel", 's', "Source Label", true);
  p.add<string>("targetLabel", 't', "Target Label", true);
  p.add("verbose", 'v', "Output debug info.");

  p.parse_check(argc, argv);
  string topicModelPath = p.get<string>("topicModel");
  string ngramVecsPath = p.get<string>("ngramVecs");
  string pmidVecsPath = p.get<string>("pmidVecs");
  string cuidVecsPath = p.get<string>("cuidVecs");
  string sourceLabel = p.get<string>("sourceLabel");
  string targetLabel = p.get<string>("targetLabel");
  ::verbose = p.exist("verbose");

  vout << "Loading topic model from " << topicModelPath << endl;
  vector<Topic> topics = getTM(topicModelPath);
  vout << "Loaded  " << topics.size() << " topics." << endl;

  vout << "Making GetVector Obj." << endl;
  GetVector getVec(ngramVecsPath, pmidVecsPath, cuidVecsPath);

  vout << "Getting all unique labels" << endl;
  unordered_set<string> labels;
  labels.insert(sourceLabel);
  labels.insert(targetLabel);
  for(const Topic & topic : topics){
    for(const auto & p : topic){
      labels.insert(p.first);
    }
  }

  vout << "Found " << labels.size() << " uniq vectors." << endl;
  vout << "Loading word vectors for each label" << endl;

  unordered_map<string, vector<float>> word2vec = getVec(labels);

  vout << "Found " << word2vec.size() << " word vectors." << endl;

  vout << "Calculating topic centers." << endl;
  vector<vector<float>> topicCentroids(topics.size());

#pragma omp parallel for
  for(unsigned int i = 0; i < topics.size(); ++i){
    topicCentroids[i] = getTopicCentroid(topics[i], word2vec);
  }

  vout << "Preparing for distance calculations" << endl;
  const vector<float>& sourceVec = word2vec[sourceLabel];
  const vector<float>& targetVec = word2vec[targetLabel];

  //float distST = distL2(sourceVec, targetVec);
  float simST = cosSim(sourceVec, targetVec);

  unsigned int matchCount = 0;
  //float minDist = numeric_limits<float>::infinity();
  float maxSim = -2;
  float minSim = 2;

  vout << "Getting distances to each topic" << endl;

#pragma omp parallel for
  for(unsigned int i = 0 ; i < topicCentroids.size(); ++i){
    const vector<float>& topicVec = topicCentroids[i];
//    float distSx = distL2(sourceVec, topicVec);
//    float distTx = distL2(targetVec, topicVec);
//    float score = distST / (distSx + distTx);
    float simSx = cosSim(sourceVec, topicVec);
    float simTx = cosSim(targetVec, topicVec);
    float score = (simSx + simTx) / 2;
//    if(max(distSx, distTx) < distST){
    if(simST < max(simSx, simTx)){
#pragma omp critical (matchCount)
      matchCount += 1;
    }

#pragma omp critical (maxSim)
    if(score > maxSim){
    //if(score < minDist){
      //minDist = score;
      maxSim = score;
    }
#pragma omp critical (minSim)
    if(score < minSim){
    //if(score < minDist){
      //minDist = score;
      minSim = score;
    }
  }

  cout << sourceLabel << " "
       << targetLabel << " "
       << simST << " "
       << matchCount / float(topics.size()) << " "
       //<< minDist << endl;
       << maxSim << " "
       << minSim << endl;

  return 0;
}
