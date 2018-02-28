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
#include"getUniqueLabels.h"
#include"util.h"
#include"files.h"

bool verbose = false;
#define vout if(::verbose) cout

unordered_map<string, nodeIdx> label2idx;

using namespace std;

edge line2edge(const string& line){
  // ensure we don't edit
  const unordered_map<string, nodeIdx>& l2i = label2idx;
  stringstream ss(line);
  string a, b;
  float w;
  ss >> a >> b >> w;
  nodeIdx aIdx = l2i.at(a);
  nodeIdx bIdx = l2i.at(b);
  if(aIdx > bIdx){
    return edge(bIdx, aIdx, w);
  } else {
    return edge(aIdx, bIdx, w);
  }
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("output", 'o', "result file, (.bin.edges, .labels)", true);
  p.add("verbose", 'v', "outputs debug information");
  p.footer("[edges weight]+");

  p.parse_check(argc, argv);

  if(p.rest().size() %2 != 0 || p.rest().size()==0){
    throw runtime_error("Must supply pairs of 'file weight'");
  }

  string outPath =  p.get<string>("output");
  verbose = p.exist("verbose");

  unordered_map<string, float> path2weight;
  for(size_t i = 0; i < p.rest().size(); i+=2){
    const string& path = p.rest()[i];
    const string& weightStr = p.rest()[i+1];
    if(!isFile(path)){
      throw runtime_error(path+" is not a valid file path");
    }
    float weight;
    try{
      weight = stof(weightStr);
    } catch (...) {
      throw runtime_error(weightStr + " is not a valid weight");
    }
    path2weight[path] = weight;
  }

  vout << "Getting Labels:" << endl;
  unordered_set<string> labelSet;
  for(const auto& pair : path2weight){
    vout << "Proc: " << pair.first << endl;
    getUniqueLabels(pair.first, labelSet);
  }
  vout << "Found " << labelSet.size() << " labels" << endl;
  vout << "Indexing & writing" << endl;
  nodeIdx idx = 0;
  fstream labelFile(outPath + ".labels", ios::out);
  for(const string& str : labelSet){
    labelFile << str << endl;
    label2idx.emplace(move(str), idx);
    ++idx;
  }
  labelFile.close();


  fstream edgeFile(outPath + ".bin.edges", ios::out|ios::binary);

  for(const auto& pair : path2weight){
    vout << "Processing " << pair.first << endl;
    list<edge> edges;

    fastProcFile<edge>(pair.first, edges, line2edge);

    float minW, maxW;
    minW = maxW = edges.begin()->weight;


    vout << "Normalizing" << endl;
    for(const auto& e : edges){
      if(e.weight > maxW) maxW = e.weight;
      if(e.weight < minW) minW = e.weight;
    }

    vout << "Writing" << endl;
    float diff = maxW - minW;
    for(auto& e : edges){
      e.weight = pair.second * (e.weight - minW) / diff;
      charBuff a, b, w;
      a.i = e.a;
      b.i = e.b;
      w.f = e.weight;
      edgeFile.write(a.buff, 4);
      edgeFile.write(b.buff, 4);
      edgeFile.write(w.buff, 4);
    }
  }

  edgeFile.close();

  return 0;
}
