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

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("topic-model", 'm', "Topic model from VIEW_FILES", true);
  p.add<string>("ngram-vecs", 'N', "ngram vector file", false, "");
  p.add<string>("pmid-vecs", 'P', "pmid vector file", false, "");
  p.add<string>("umls-vecs", 'U', "cuid vector file", false, "");
  p.add<string>("source-label", 's', "Source Label", true);
  p.add<string>("target-label", 't', "Target Label", true);
  p.add("verbose", 'v', "Output debug info.");

  p.parse_check(argc, argv);
  string topicModelPath = p.get<string>("topic-model");
  string ngramVecsPath = p.get<string>("ngram-vecs");
  string pmidVecsPath = p.get<string>("pmid-vecs");
  string cuidVecsPath = p.get<string>("umls-vecs");
  string sourceLabel = p.get<string>("source-label");
  string targetLabel = p.get<string>("target-label");
  ::verbose = p.exist("verbose");

  vout << "Loading topic model from " << topicModelPath << endl;
  vector<Topic> topics = getTM(topicModelPath);
  vout << "Loaded  " << topics.size() << " topics." << endl;

  vout << "Making GetVector Obj." << endl;
  GetVector getVec(ngramVecsPath, pmidVecsPath, cuidVecsPath);

  vout << "Getting word2topics" << endl;

  unordered_map<string, vector<float>> word2topics;
  unordered_set<string> labels; // all labels
  labels.insert(sourceLabel);
  labels.insert(targetLabel);

  for(unsigned int topicIdx = 0; topicIdx < topics.size(); ++topicIdx){
    const Topic& topic = topics[topicIdx];
    for(const auto& pair : topic){
      if(word2topics.find(pair.first) == word2topics.end()){
        labels.insert(pair.first);
        word2topics[pair.first] = vector<float>(topics.size(), 0);
      }
      word2topics[pair.first][topicIdx] = pair.second;
    }
  }

  vout << "Found " << labels.size() << " uniq labels." << endl;

//  vout << "Normalizing all word2topic vectors" << endl;
//
//  for(auto& pair : word2topics){
//    vector<float>& countByTopics = pair.second;
//    float sum = 0;
//    for(float f : countByTopics)
//      sum += f;
//    countByTopics /= sum;
//  }

  vout << "Loading word vectors for each label" << endl;

  unordered_map<string, vector<float>> word2vec = getVec(labels);

  vector<float> sourceVec = word2vec[sourceLabel];
  vector<float> targetVec = word2vec[targetLabel];

  vector<float> sourceByTopic(topics.size(), 0);
  vector<float> targetByTopic(topics.size(), 0);
  vector<float> sumByTopics(topics.size(), 0);

  vout << "Found " << word2vec.size() << " word vectors." << endl;

  vout << "Computing similarity measues for each topic" << endl;


  //float count = 0;
  for(const auto& pair : word2vec){
    const string& word = pair.first;
    const vector<float>& vec = pair.second;

    if(word2topics.find(word) != word2topics.end()){
      float simA = cosSim(sourceVec, vec);
      float simB = cosSim(targetVec, vec);
      const vector<float>& wordTopics = word2topics[word];
      sourceByTopic += wordTopics * simA;
      targetByTopic += wordTopics * simB;
      sumByTopics += wordTopics;
  //    count += 1;
    }
  }

  for(unsigned int i = 0 ; i < topics.size(); ++i){
    if(sumByTopics[i] > 0){
      sourceByTopic[i] /= sumByTopics[i];
      targetByTopic[i] /= sumByTopics[i];
    }
  }

//  sourceByTopic /= magnitude(sourceByTopic);
//  targetByTopic /= magnitude(targetByTopic);

  vout << "Topic" << "\t" << "Source" << "\t" << "Target" << endl;
  for(unsigned int i = 0; i < topics.size(); ++i){
    vout << i << "\t" << sourceByTopic[i] << "\t" << targetByTopic[i] << endl;
  }

  float score = cosSim(sourceByTopic, targetByTopic);

  cout << sourceLabel << " "
       << targetLabel << " "
       << score << endl;

  return 0;
}
