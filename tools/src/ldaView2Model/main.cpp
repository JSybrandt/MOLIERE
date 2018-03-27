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

#include"cmdln.h"
#include"util.h"
#include"topic_model.h"

using namespace std;

bool verbose = false;
#define vout if(::verbose) cout

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input", 'i', "input graph (BINARY) file", true);
  p.add<string>("output", 'o', "Label file accompanying the edges file.", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string inPath = p.get<string>("input");
  string outPath = p.get<string>("output");
  ::verbose = p.exist("verbose");

  vector<Topic> topics = getTM(inPath);

  // I want it ordered
  map<string, vector<size_t>> word2counts;

  for(size_t topicIdx = 0; topicIdx < topics.size(); ++topicIdx){
    for(const auto& p : topics[topicIdx]){
      const string& word = p.first;
      const size_t count = p.second;

      if(word2counts.find(word) == word2counts.end()){
        word2counts[word] = vector<size_t>(topics.size(), 0);
      }
      word2counts[word][topicIdx] = count;
    }
  }

  fstream outFile(outPath, ios::out);
  for(const auto& p : word2counts){
    outFile << p.first;
    for(size_t c : p.second){
      outFile << " " << c;
    }
    outFile << endl;
  }
  outFile.close();

}
