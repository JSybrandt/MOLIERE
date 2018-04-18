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

rawEdge line2raw(const string& line){
  stringstream ss(line);
  string a, b;
  float w;
  ss >> a >> b >> w;
  return {a, b, w};
}


vector<float> getWeightedCentroid(const list<pair<string, float>>& neighborData,
                                  const unordered_map<string, vector<float>>& word2vec){
  // result is of veclength (determnied by length of rand vec) and starts all 0
  vector<float> res(word2vec.begin()->second.size(), 0.0f);

  float weightSum = 0;
  for(const auto& p : neighborData){
    // REMEBER! the network edge weight is inverse TF-IDF
    // Warning, float accuracy?
    float weight = 1/p.second;
    res += word2vec.at(p.first) * weight;
    weightSum += weight;
  }
  res /= weightSum;
  return res;
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input", 'i', "Edge file, each line is an undirected connection with weight", true);
  p.add<string>("vectors", 'V', "each line of this file is a known vector", true);
  p.add<string>("output", 'o', "output vec file for unknown nodes", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string inPath =  p.get<string>("input");
  string vectorPath =  p.get<string>("vectors");
  string outPath =  p.get<string>("output");
  verbose = p.exist("verbose");

  vout << "Loading Vectors" << endl;

  list<pair<string, vector<float>>> word2vecList;
  fastProcFile<pair<string, vector<float>>>(vectorPath, word2vecList, line2vec);
  vout << "List 2 map" << endl;
  unordered_map<string, vector<float>> word2vec;
  word2vec.reserve(word2vecList.size());
  for(auto& p : word2vecList){
    word2vec.emplace(move(p));
  }

  vout << "Loading graph" << endl;
  list<rawEdge> rawEdgeList;
  fastProcFile<rawEdge>(inPath, rawEdgeList, line2raw);
  vout << "List 2 vec" << endl;
  vector<rawEdge> rawEdges;
  rawEdges.reserve(rawEdgeList.size());
  for(auto& e : rawEdgeList){
    rawEdges.emplace_back(move(e));
  }

  // map unknown nodes to neighbors based on tfidf
  // list needed for easy parallel merging
  unordered_map<string, list<pair<string, float>>> unknown2neghbors;
  vector<string> unknownNames;

  fstream outFile(outPath, ios::out);

#pragma omp parallel
  {
    unordered_map<string, list<pair<string, float>>> localData;
    #pragma omp for schedule(dynamic) nowait
    for(size_t i = 0 ; i < rawEdges.size(); ++i){
      rawEdge& e = rawEdges[i];
      string& a = get<0>(e);
      string& b = get<1>(e);
      float w = get<2>(e);
      bool haveA = word2vec.find(a) != word2vec.end();
      bool haveB = word2vec.find(b) != word2vec.end();

      // we only want to calculate if we know one and can use it for the other
      if(haveA == haveB)
        continue;
      if(haveA){
        //work under the assumption that B is known, and A is unknown
        swap(a, b);
      }
      // append information about new connection
      localData[a].push_back({b, w});
    }
    #pragma omp critical
    {
      for(auto& p : localData){
        const string& w = p.first;
        list<pair<string, float>>& l = p.second;
        // note: this will make a dummy list if non-exists
        list<pair<string, float>>& old = unknown2neghbors[w];
        // splice the local list into the overall list
        old.splice(old.end(), l);
      }
    }

    #pragma omp barrier

    #pragma omp single
    {
      vout << "indexing " << unknown2neghbors.size() << " unknown nodes." << endl;
      unknownNames.reserve(unknown2neghbors.size());
      for(const auto& p : unknown2neghbors){
        if(p.second.size() > 0)
          unknownNames.emplace_back(p.first);
      }
      vout << "Calculating and Writing..." << endl;
    }

    #pragma omp barrier

    #pragma omp for schedule(dynamic)
    for(size_t i = 0 ; i < unknownNames.size(); ++i){
      const string& name = unknownNames[i];
      const list<pair<string, float>>& neighborList = unknown2neghbors[name];
      const vector<float>& vec = getWeightedCentroid(neighborList, word2vec);
      #pragma omp critical
      {
        outFile << name << " " << vec << endl;
      }
    }
  }

  outFile.close();

  return 0;
}
