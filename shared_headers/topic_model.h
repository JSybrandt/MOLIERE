#include<string>
#include<vector>
#include<fstream>
#include<unordered_map>

#include"util.h"

using std::string;
using std::vector;
using std::fstream;
using std::unordered_map;
using std::pair;
using std::ios;

typedef vector<pair<string, unsigned int>> Topic;

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

  return result;
}

vector<float> getCentroid(const Topic & topic,
                          const unordered_map<string, vector<float>> & word2vec){
  vector<float> res(word2vec.begin()->second.size(), 0);
  float totalCount = 0;
  for(const auto & pair : topic){
    const string& word = pair.first;
    float occuranceCount = pair.second;
    const auto it = word2vec.find(word);
    if(it != word2vec.end()){
      res += it->second * occuranceCount;
      totalCount += occuranceCount;
    }
  }
  res /= float(totalCount);
  return res;

}
