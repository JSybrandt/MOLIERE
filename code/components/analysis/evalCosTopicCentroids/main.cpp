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
#include<algorithm>
#include"cmdln.h"
#include"util.h"
#include"topic_model.h"

using namespace std;

// an array of lists of pairs
typedef vector<pair<string, unsigned int>> Topic;

bool verbose;
#define vout if(::verbose) cout

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("topicModel", 'm', "Topic model from VIEW_FILES", true);
  p.add<string>("ngramVecs", 'n', "ngram vector file", false, "");
  p.add<string>("pmidVecs", 'p', "pmid vector file", false, "");
  p.add<string>("cuidVecs", 'c', "cuid vector file", false, "");
  p.add<string>("sourceLabel", 's', "Source Label", true);
  p.add<string>("targetLabel", 't', "Target Label", true);
  p.add("verbose", 'v', "Output debug info.");
  p.add("euclidian", 'e', "Use euclidian distance instead of cosine similarity.");
  p.add<unsigned int>("numReportedTopics", 'r', "Number of topics to report", false, 5);
  p.add<unsigned int>("topicCutoff", 'f', "number of words to be reported per topic", false, 5);

  p.parse_check(argc, argv);
  string topicModelPath = p.get<string>("topicModel");
  string ngramVecsPath = p.get<string>("ngramVecs");
  string pmidVecsPath = p.get<string>("pmidVecs");
  string cuidVecsPath = p.get<string>("cuidVecs");
  string sourceLabel = p.get<string>("sourceLabel");
  string targetLabel = p.get<string>("targetLabel");
  ::verbose = p.exist("verbose");
  bool euclidian = p.exist("euclidian");
  unsigned int numReportedTopics = p.get<unsigned int>("numReportedTopics");
  unsigned int topicCutoff = p.get<unsigned int>("topicCutoff");

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
  vector<float> topicScores(topics.size(), 0);

#pragma omp parallel for
  for(unsigned int i = 0; i < topics.size(); ++i){
    topicCentroids[i] = getCentroid(topics[i], word2vec);
  }

  vout << "Preparing for distance calculations" << endl;
  const vector<float>& sourceVec = word2vec[sourceLabel];
  const vector<float>& targetVec = word2vec[targetLabel];
  vector<float> middle; // used only in the euclidian case

  float scoreST;
  if(euclidian){
    scoreST = distL2(sourceVec, targetVec);
    middle = (sourceVec + targetVec) * 0.5;
  } else {
    scoreST = cosSim(sourceVec, targetVec);
  }

  float maxScore = -numeric_limits<float>::infinity();
  float minScore = numeric_limits<float>::infinity();

  vout << "Getting distances to each topic" << endl;

#pragma omp parallel for
  for(unsigned int i = 0 ; i < topicCentroids.size(); ++i){
    const vector<float>& topicVec = topicCentroids[i];
    float &score = topicScores[i];
    if(euclidian){
      float dist2Mid = distL2(topicVec, middle);
      score = max(1 - (dist2Mid / (scoreST/2)), 0.0f);
    } else {
      float simSx = cosSim(sourceVec, topicVec);
      float simTx = cosSim(targetVec, topicVec);
      score = (simSx + simTx) / 2;
    }
#pragma omp critical (maxScore)
    if(score > maxScore){
      maxScore = score;
    }
#pragma omp critical (minScore)
    if(score < minScore){
      minScore = score;
    }
  }

  // print scores
  cout << sourceLabel << " "
       << targetLabel << " "
       << scoreST << " "
       << maxScore << " "
       << minScore;

  vector<pair<float, unsigned int>> score2topic(topics.size());
  for(unsigned int i = 0 ; i < topics.size(); ++i){
      score2topic.push_back(pair<float, unsigned int>(topicScores[i], i));
  }
  sort(score2topic.begin(), score2topic.end(), cmpRev<float, unsigned int>);

  for(unsigned int i = 0;
      i < min((unsigned int) topics.size(), numReportedTopics);
      ++i)
  {
    auto& pair = score2topic[i];
    Topic& topic = topics[pair.second];
    cout << " TOPIC" << pair.second << " " << pair.first;
    for(unsigned int i = 0;
        i < min(topicCutoff, (unsigned int) topic.size());
        ++i){
      cout << " " << topic[i].first;
    }
  }

  cout << endl;

  return 0;
}
