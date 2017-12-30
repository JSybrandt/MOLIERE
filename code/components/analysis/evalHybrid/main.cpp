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
#include"weighted_kmeans.h"
#include<omp.h>

using namespace std;

bool verbose;
#define vout if(::verbose) cout

bool euclidian;

const unsigned int NUM_METRICS = 4;

float getScorePerWord(const Topic& topic,
                      const unordered_map<string, vector<float>>& word2vec,
                      const vector<float>& sourceVec,
                      const vector<float>& targetVec,
                      float distST){
  distST /= 2; // really just want dist 2 midpoint
  float coveredCount = 0;
  float scoreS = 0, scoreT = 0;
  for(const auto& pair : topic){
    const string& word = pair.first;
    const float& count = pair.second;

    // If the word is covered in the embedding space
    if(word2vec.find(word) != word2vec.end()){
      const vector<float>& vec = word2vec.at(word);
      coveredCount += count;

      if(::euclidian){
        scoreS += count * max(1 - (distL2(sourceVec, vec)/distST), 0.0f);
        scoreT += count * max(1 - (distL2(targetVec, vec)/distST), 0.0f);
      } else {
        scoreS += count * max(cosSim(sourceVec, vec), 0.0f);
        scoreT += count * max(cosSim(targetVec, vec), 0.0f);
      }
    }
  }
  if(coveredCount > 0){
    scoreS /= coveredCount;
    scoreT /= coveredCount;
  }
  return max(scoreS, scoreT);
}

float getScorePerCluster(const Topic& topic,
                         const unordered_map<string, vector<float>>& word2vec,
                         const vector<float>& sourceVec,
                         const vector<float>& targetVec,
                         float distST){

  vector<pair<vector<float>,float>> weightedClusterData;
  weightedClusterData.reserve(topic.size());
  for(const auto& p : topic){
    const string& str = p.first;
    float weight = p.second;
    if(word2vec.find(str) != word2vec.end()){
      vector<float> vec = word2vec.at(str);
      weightedClusterData.push_back(pair<vector<float>,float>(vec, weight));
    }
  }
  unsigned int k = 2;
  KMeansRes res = weightedKmeans(weightedClusterData, k);

  vector<float> scoreK2S(k, 0.0f), scoreK2T(k, 0.0f);
  for(unsigned int i = 0; i < k; ++i){
    if(euclidian){
      scoreK2S[i] = max(1 - distL2(res.centroids[i], sourceVec) / distST, 0.0f);
      scoreK2T[i] = max(1 - distL2(res.centroids[i], targetVec) / distST, 0.0f);
    } else {
      scoreK2S[i] = max(cosSim(res.centroids[i], sourceVec), 0.0f);
      scoreK2T[i] = max(cosSim(res.centroids[i], targetVec), 0.0f);
    }
  }

  float score = 0;
  //need to find two centroids that have the best score
  //note, written this way just to let k be flexible later
  for(unsigned int i = 0; i < k; ++i){
    for(unsigned int j = 0; j < k; ++j){
      if(j != i){ // I want to take all permuations of 2
        score = max(score, (scoreK2S[i] + scoreK2T[j])/2);
      }
    }
  }
  return score;
}

float getScoreForCentroid(const Topic& topic,
                          const unordered_map<string, vector<float>>& word2vec,
                          const vector<float>& sourceVec,
                          const vector<float>& targetVec,
                          const vector<float>& middleVec,
                          const float distST){
  float score;
  vector<float> topicVec = getCentroid(topic, word2vec);
  if(euclidian){
    float dist2Mid = distL2(topicVec, middleVec);
    score = max(1 - (dist2Mid / (distST/2)), 0.0f);
  } else {
    float simSx = max(cosSim(sourceVec, topicVec), 0.0f);
    float simTx = max(cosSim(targetVec, topicVec), 0.0f);
    score = (simSx + simTx) / 2;
  }
  return score;
}

vector<pair<float, float>> parseWeightingParamStr(const string& params){
  stringstream ss(params);
  vector<pair<float, float>> res;
  float coef, exp;
  while(ss >> coef >> exp){
    res.push_back(pair<float, float>(coef, exp));
    vout << "C: " << coef << "\tE: " << exp << endl;
  }
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
  p.add("max", 'M', "Take max of metrics, rather than a combination.");
  p.add("euclidian", 'e', "Use euclidian distance instead of cosine similarity.");
  p.add<unsigned int>("numReportedTopics", 'r', "Number of topics to report", false, 5);
  p.add<unsigned int>("topicCutoff", 'f', "number of words to be reported per topic", false, 5);
  p.add<string>("weightParamStr", 'P', "4 pairs of coef and power", false,
                "1 1  1 1  1 1  1 1");

  p.parse_check(argc, argv);
  string topicModelPath = p.get<string>("topicModel");
  string ngramVecsPath = p.get<string>("ngramVecs");
  string pmidVecsPath = p.get<string>("pmidVecs");
  string cuidVecsPath = p.get<string>("cuidVecs");
  string sourceLabel = p.get<string>("sourceLabel");
  string targetLabel = p.get<string>("targetLabel");
  ::verbose = p.exist("verbose");
  bool takeMax = p.exist("max");
  ::euclidian = p.exist("euclidian");
  unsigned int numReportedTopics = p.get<unsigned int>("numReportedTopics");
  unsigned int topicCutoff = p.get<unsigned int>("topicCutoff");
  string weightParamStr = p.get<string>("weightParamStr");

  vout << "Parsing Weighting Param" << endl;
  vector<pair<float, float>> weightParams = parseWeightingParamStr(weightParamStr);
  if(weightParams.size() != NUM_METRICS){
    cerr << "ILLEGAL PARSE STRING." << endl;
    exit(1);
  }

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

  vout << "Loaded " <<  word2vec.size() << " vectors" << endl;

  if(word2vec.find(sourceLabel) != word2vec.end() &&
     word2vec.find(targetLabel) != word2vec.end()){

    vout << "Found " << word2vec.size() << " word vectors." << endl;

    vector<float> topicScores(topics.size(), 0);

    vout << "Preparing for calculations" << endl;

    const vector<float>& sourceVec = word2vec.at(sourceLabel);
    const vector<float>& targetVec = word2vec.at(targetLabel);
    const vector<float>& middleVec = (sourceVec + targetVec) * 0.5;
    float distST = distL2(sourceVec, targetVec);
    float simST  = cosSim(sourceVec, targetVec);
    float scoreST;
    if(euclidian)
      scoreST = 1 - distST / float(2 * sqrt(sourceVec.size()));
    else
      scoreST = simST;

  #pragma omp parallel for
    for(unsigned int i = 0 ; i < topics.size(); ++i){
      const Topic& topic = topics[i];
      float scores[4] = {
        scoreST,
        getScoreForCentroid(topic, word2vec, sourceVec, targetVec, middleVec, distST),
        getScorePerCluster(topic, word2vec, sourceVec, targetVec, distST),
        getScorePerWord(topic, word2vec, sourceVec, targetVec, distST)
      };
      float &score = topicScores[i];
      if(takeMax){
        score = -numeric_limits<float>::infinity();
        for(unsigned int j = 0; j < NUM_METRICS; ++j){
          score = max(score, weightParams[j].first * pow(scores[j], weightParams[j].second));
        }
      } else {
        for(unsigned int j = 0; j < NUM_METRICS; ++j){
          score += weightParams[j].first * pow(scores[j], weightParams[j].second);
        }
      }
    }

    float bestScore = *max_element(topicScores.begin(), topicScores.end());

    // print scores
    cout << sourceLabel << " "
         << targetLabel << " "
         << bestScore;

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
  } else {
    cerr << "FAILED TO FIND ";
    if(word2vec.find(sourceLabel) == word2vec.end())
      cerr << sourceLabel << " ";
    if(word2vec.find(targetLabel) == word2vec.end())
      cerr << targetLabel;
    cerr << endl;
  }

  return 0;
}
