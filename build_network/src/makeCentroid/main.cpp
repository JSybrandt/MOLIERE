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

#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"cmdln.h"
#include"parallelFileOp.h"
#include"util.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

unordered_map<string, vector<float>> word2vec;
bool skipSecond = false;

pair<string, vector<float>> text2centroid(const string& line){
  const auto& w2v = word2vec;  // ensure we don't modify
  string tmp;
  float count = 0;
  pair<string, vector<float>> res;
  stringstream reader(line);
  reader >> res.first;
  if(skipSecond) reader >> tmp;
  while(reader >> tmp){
    if(w2v.find(tmp) != w2v.end()){
      if(count == 0)
        res.second = w2v.at(tmp);
      else
        res.second += w2v.at(tmp);
      count += 1;
    }
  }
  res.second /= count;
  return res;
}


// ensures that every record we look at has a vector
bool containsWord(const string& line){
  const auto& w2v = word2vec;  // ensure we don't modify
  string tmp;
  stringstream reader(line);
  reader >> tmp;  // skip label
  if(skipSecond) reader >> tmp;
  while(reader >> tmp){
    if(w2v.find(tmp) != w2v.end()){
      return true;
    }
  }
  return false;
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input-file", 'i', "text lines, first word is label", true);
  p.add<string>("vector-file", 'V', "each line of this file is a vector", true);
  p.add<string>("output-file", 'o', "output file", true);
  p.add("skip-second", 's', "skips second token, i.e. year in the ab file");
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string inputPath =  p.get<string>("input-file");
  string vectorPath =  p.get<string>("vector-file");
  string outPath =  p.get<string>("output-file");
  verbose = p.exist("verbose");
  skipSecond = p.exist("skip-second");

  vout << "Loading Vectors" << endl;

  list<pair<string, vector<float>>> word2vecList;
  fastProcFile<pair<string, vector<float>>>(vectorPath, word2vecList, line2vec);
  word2vec.insert(make_move_iterator(word2vecList.begin()),
                  make_move_iterator(word2vecList.end()));

  vout << "Making Centroids" << endl;
  list<pair<string, vector<float>>> label2centroidList;
  fastProcFile<pair<string, vector<float>>>(inputPath, label2centroidList,
                                            text2centroid, containsWord);

  vout << "writing" << endl;
  fstream outFile(outPath, ios::out);
  for(const auto& pair : label2centroidList){
    outFile << pair.first << " " << pair.second << endl;
  }
  outFile.close();

  return 0;
}
