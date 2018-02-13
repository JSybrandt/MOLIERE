#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<string>
#include<vector>
#include<queue>
#include<limits>
#include<fstream>
#include<stack>
#include<sstream>
#include<exception>
#include<list>

#include<omp.h>

#include"cmdln.h"
#include"pQueue.h"
#include"graph.h"
#include"util.h"
#include"topic_model.h"
#include"parallelAbstractLoad.h"

bool verbose = false;
#define vout if(::verbose) cout

vector<float> bow2TopicMix(const string bow, const unordered_map<string, vector<float>> word2mix){
  // in the mixute, all words must have the same num topics vector
  unsigned int num_topics = word2mix.begin()->second.size();
  vector<float> mixtures(num_topics, 0.0f);
  stringstream ss(bow);
  string token;
  unsigned int count;
  while(ss >> token >> count){
    if(word2mix.find(token) != word2mix.end()){
      mixtures += word2mix.at(token) * count;
    }
  }
  return mixtures;
}

unordered_map<string, vector<float>> word2TopicMix(const vector<Topic>& tm){
  vector<float> countPerTopic(tm.size(), 0.0f);
  for(unsigned int i = 0; i < tm.size(); i++){
    for(pair<string, unsigned int> p : tm[i]){
      countPerTopic[i] += p.second;
    }
  }
  unordered_map<string, vector<float>> word2mix;
  for(unsigned int i = 0; i < tm.size(); i++){
    for(pair<string, unsigned int> p: tm[i]){
      if(word2mix.find(p.first) == word2mix.end()){
        word2mix[p.first] = vector<float>(tm.size(), 0.0f);
      }
      word2mix[p.first][i] = float(p.second) / countPerTopic[i];
    }
  }
  return word2mix;
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("topic_model", 't', "topic model file (view)", true);
  p.add<string>("abstract_file", 'a', "abstract file, first word is PMID", true);
  p.add<string>("cloud_file", 'c', "cloud file, from the query.", true);
  p.add<string>("label_file", 'l', "label file, each line is an id", true);
  p.add<unsigned int>("topic_num", 'T', "topic number of which to return results", true);
  p.add<unsigned int>("result_num", 'n', "number of papers to retutrn", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string topicModelPath =  p.get<string>("topic_model");
  string labelPath =  p.get<string>("label_file");
  string abstractPath =  p.get<string>("abstract_file");
  string cloudPath =  p.get<string>("cloud_file");
  unsigned int topicNum = p.get<unsigned int>("topic_num");
  unsigned int resultNum = p.get<unsigned int>("result_num");
  verbose = p.exist("verbose");

  vout << "Loading labels from " << labelPath << endl;
  vector<string> labels;
  fstream lFile(labelPath, ios::in);
  string tmp;
  while(lFile >> tmp){
    labels.push_back(tmp);
  }
  lFile.close();

  vout << "Getting PMIDS from " << cloudPath << endl;
  unordered_set<string> pmids;
  fstream cFile(cloudPath, ios::in);
  nodeIdx idx;
  while(cFile >> idx){
    if(labels[idx].substr(0, 4) != "PMID"){
      vout << "Skipping " << labels[idx] << ", it isn't a PMID" << endl;
    } else {
      pmids.insert(labels[idx]);
    }
  }
  cFile.close();

  vector<Topic> topicModel = getTM(topicModelPath);
  Topic selectedTopic = topicModel[topicNum];
  unordered_map<string, float> word2count(selectedTopic.begin(), selectedTopic.end());


  vout << "Loading from " << abstractPath << " in parallel" << endl;
  list<pair<string, string>> bowList;
  fastLoadAbstract2Bow(abstractPath, bowList, pmids);
  vector<pair<string, string>> bowVec {
    make_move_iterator(bowList.begin()),
    make_move_iterator(bowList.end())
  };
  vout << "Loaded " << bowList.size() << " pmids" << endl;

  // output goes here
  vector<float> scores(bowVec.size(), 0.0f);

  vout << "Finding the mixtures" << endl;

#pragma omp parallel for
  for(unsigned int i = 0; i < bowVec.size(); ++i){
    const pair<string, string> pr = bowVec[i];
    const string& bow = pr.second;
    float& score = scores[i];
    stringstream ss(bow);
    string token;
    unsigned int count;
    while(ss >> token >> count){
      if(word2count.find(token) != word2count.end()){
        score += word2count.at(token) * count;
      }
    }
  }

  vout << "Selecting the best" << endl;
  pQueue<string, float> bestAbstracts;
  for(unsigned int i = 0; i < bowVec.size(); ++i){
    const string& pmid = bowVec[i].first;
    float score = scores[i];
    // push in new, hopefully large, values
    bestAbstracts.push(pmid, score);
    // pop off old, smaller values
    while(bestAbstracts.size() > resultNum) bestAbstracts.pop();
  }

  while(bestAbstracts.size() > 0){
    pair<string, float> pr = bestAbstracts.pop();
    cout << pr.first << " " << pr.second << endl;
  }


}
