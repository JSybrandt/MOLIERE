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

using namespace std;

// an array of lists of pairs
typedef vector<pair<string, unsigned int>> Topic;

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
      result[currTopic].push_back(pair<string, unsigned int>(token, occuranceRate));
    }
  }
  tmFile.close();

  //if(::verbose){
    //for(unsigned int i = 0; i < result.size(); ++i){
      //vout << "TOPIC " << i << endl;
      //for(auto& p : result[i]){
        //vout << "\t" << p.first << " " << p.second << endl;
      //}
    //}
  //}

  return result;
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
  p.add("euclidian", 'e', "Use euclidian distance instead of cosine similarity.");
  p.add("betterScore", 'B', "Take the best score between scoreS and scoreT");
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
  bool betterScore = p.exist("betterScore");
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

  vector<float> topicScores(topics.size(), 0);

  vout << "Preparing for calculations" << endl;
  const vector<float>& sourceVec = word2vec.at(sourceLabel);
  const vector<float>& targetVec = word2vec.at(targetLabel);
  float distST = distL2(sourceVec, targetVec);

#pragma omp parallel for
  for(unsigned int i = 0 ; i < topics.size(); ++i){
    const Topic& topic = topics[i];
    float coveredCount = 0;
    float scoreS = 0, scoreT = 0;

    for(const auto& pair : topic){
      const string& word = pair.first;
      const float& count = pair.second;

      vout << "Considering " << word << endl;

      // If the word is covered in the embedding space
      if(word2vec.find(word) != word2vec.end()){
        vout << "\tTRUE" << endl;
        const vector<float>& vec = word2vec.at(word);
        coveredCount += count;

        if(euclidian){
          scoreS += count * max(1 - (distL2(sourceVec, vec)/distST), 0.0f);
          scoreT += count * max(1 - (distL2(targetVec, vec)/distST), 0.0f);
        } else {
          scoreS += count * max(cosSim(sourceVec, vec), 0.0f);
          scoreT += count * max(cosSim(targetVec, vec), 0.0f);
        }
      } else {
        vout << "\tFALSE"<< endl;
      }
    }
    if(coveredCount > 0){
      scoreS /= coveredCount;
      scoreT /= coveredCount;
    }
    if(betterScore)
      topicScores[i] = max(scoreS, scoreT);
    else
      topicScores[i] = min(scoreS, scoreT);
  }

  // print scores
  cout << sourceLabel << " "
       << targetLabel << " "
       << *max_element(topicScores.begin(), topicScores.end());

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
