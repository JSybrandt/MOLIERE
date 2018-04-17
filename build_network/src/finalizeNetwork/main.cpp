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
#include <unistd.h> // for truncate

#include"cmdln.h"
#include"parallelFileOp.h"
#include"getUniqueLabels.h"
#include"appendParallelEdges.h"
#include"util.h"
#include"files.h"

bool verbose = false;
#define vout if(::verbose) cout


using namespace std;

typedef tuple<string, string, float> rawEdge;

rawEdge line2raw(const string& line){
  stringstream ss(line);
  string a, b;
  float w;
  ss >> a >> b >> w;
  return {a, b, w};
}

edge raw2edge(const rawEdge& raw, const unordered_map<string, nodeIdx>& label2idx){
  nodeIdx a = label2idx.at(get<0>(raw));
  nodeIdx b = label2idx.at(get<1>(raw));
  float w = get<2>(raw);
  return edge(a, b, w);
}

void addLabel(const string& str, unordered_map<string, nodeIdx>& label2idx){
  if(label2idx.find(str) == label2idx.end()){
    label2idx[str] = label2idx.size();
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


  //vout << "Getting Labels:" << endl;
  //unordered_set<string> labelSet;
  //for(const auto& pair : path2weight){
    //vout << "Proc: " << pair.first << endl;
    //getUniqueLabels(pair.first, labelSet);
  //}
  //vout << "Found " << labelSet.size() << " labels" << endl;
  //vout << "Indexing & writing" << endl;
  //nodeIdx idx = 0;
  //fstream labelFile(outPath + ".labels", ios::out);
  //for(const string& str : labelSet){
    //labelFile << str << endl;
    //label2idx.emplace(move(str), idx);
    //++idx;
  //}
  //labelFile.close();


  // clear out file
  //
  string edgePath = outPath + ".bin.edges";
  fstream edgeFile(edgePath, ios::out|ios::binary);

  unordered_map<string, nodeIdx> label2idx;

  for(const auto& pathWeight : path2weight){
    const string& path = pathWeight.first;
    float weight = pathWeight.second;
    vout << "Loading " << path << endl;
    list<rawEdge> rawEdgeList;
    fastProcFile<rawEdge>(path, rawEdgeList, line2raw);
    vout << "List 2 vec" << endl;
    vector<rawEdge> rawEdgeVec;
    rawEdgeVec.reserve(rawEdgeList.size());
    for(rawEdge& e : rawEdgeList){
      rawEdgeVec.emplace_back(move(e));
    }

    vout << "Collecting new labels" << endl;
    // warning! can't move here. Need for another pass with raw edges
    #pragma omp parallel
    {
      unordered_set<string> localLabels;
      #pragma omp for schedule(dynamic) nowait
      for(size_t i = 0; i < rawEdgeVec.size(); ++i){
        localLabels.insert(get<0>(rawEdgeVec[i]));
        localLabels.insert(get<1>(rawEdgeVec[i]));
      }
      #pragma omp critical
      {
        for(const string& s : localLabels){
          addLabel(s, label2idx);
        }
      }
    }
    vout << "Found " << label2idx.size() << " labels" << endl;

    vout << "Raw edges to edges" << endl;
    vector<edge> edges(rawEdgeVec.size());
    #pragma omp parallel for
    for(size_t i = 0; i < rawEdgeVec.size(); ++i){
      edges[i] = raw2edge(rawEdgeVec[i], label2idx);
    }

    vout << "Normalizing" << endl;
    float minW, maxW;
    minW = maxW = edges.begin()->weight;
    #pragma omp parallel
    {
      float localMin, localMax;
      localMin = localMax = minW;
      #pragma omp for nowait
      for(size_t i = 0; i < edges.size(); ++i){
        float w = edges[i].weight;
        if(w > localMax) localMax = w;
        if(w < localMin) localMin = w;
      }
      #pragma omp critical
      {
        if(localMin < minW) minW = localMin;
        if(localMax > maxW) maxW = localMax;
      }
    }


    float diff = maxW - minW;
    vout << "Scaling all edges" << endl;
    #pragma omp parallel for
    for(size_t i = 0; i < edges.size(); ++i){
      edge& e = edges[i];
      e.weight = weight * (e.weight - minW) / diff;
    }


    vout << "Writing" << endl;
    //appendParallelEdges(edgePath, edges);
    for(edge& e : edges){
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
