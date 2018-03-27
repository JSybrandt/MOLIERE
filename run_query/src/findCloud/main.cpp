
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
#include"pQueue.h"
#include"graph.h"
#include"util.h"
#include"parallelEdgeListLoad.h"
#include"labelManager.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

unordered_set<nodeIdx> getCloud(const vector<nodeIdx>& path,
                                const Graph& graph,
                                const function<bool(nodeIdx)> isAbstract,
                                size_t minPerNode,
                                bool& needsMoreNeighbors){
  vout << "Getting Cloud" << endl;
  unordered_set<nodeIdx> cloud;
  #pragma omp parallel for
  for(size_t i = 0; i < path.size(); ++i){
    unordered_set<nodeIdx> localCloud;
    localCloud = graph.getSelectNearby(path[i], minPerNode, isAbstract);
    if(localCloud.size() < minPerNode) needsMoreNeighbors = true;
    #pragma omp critical
    cloud.insert(localCloud.begin(), localCloud.end());
  }
  return cloud;
}

int main (int argc, char** argv){

  unordered_set<char> abstractTags = {'P'};

  cmdline::parser p;

  p.add<string>("graph", 'g', "input graph (BINARY) file", true);
  p.add<string>("path", 'p', "input path file (idx)", true);
  p.add<string>("output", 'o', "Output paths and neighborhoods", true);
  p.add<string>("labels", 'l', "Label file accompanying the edges file.", true);
  p.add<unsigned int>("abstracts-per-node", 'A', "The cloud will expand until it find this many abstracts per node in path", false, 3000);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graph");
  string pathPath =  p.get<string>("path");
  string outputPath =  p.get<string>("output");
  string labelPath =  p.get<string>("labels");
  unsigned int numAbstractsPerNode = p.get<unsigned int>("abstracts-per-node");
  verbose = p.exist("verbose");

  vout << "Loading Path" << endl;
  unordered_set<nodeIdx> allNodes;
  vector<nodeIdx> path;
  fstream pathFile(pathPath, ios::in);
  nodeIdx node;
  while(pathFile >> node){
    allNodes.insert(node);
    path.push_back(node);
  }
  pathFile.close();

  // CONSTRUCTING GRAPH
  vout << "Loading labels from " << labelPath << endl;
  LabelManager labels(labelPath);

  const auto& isAbstract = [&labels, &abstractTags](nodeIdx node) -> bool{
    char tag = labels[node][0];
    return abstractTags.find(tag) != abstractTags.end();
  };

  Graph graph(graphPath, allNodes);
  unordered_set<nodeIdx> cloud;

  list<edge> edgeList;
  bool needMoreNeighbors = false;

  cloud = getCloud(path, graph,
                   isAbstract,
                   numAbstractsPerNode,
                   needMoreNeighbors);

  if(needMoreNeighbors){
    vout << "Loading More Neighbors" << endl;
    allNodes = graph.getNodes();
    graph = Graph(graphPath, allNodes);
    cloud = getCloud(path, graph,
                     isAbstract,
                     numAbstractsPerNode,
                     needMoreNeighbors);
  }

  fstream outFile(outputPath, ios::out);

  vout << "Writing to file" << endl;
  for(nodeIdx n : cloud)
    outFile << n << endl;

  outFile.close();
  return 0;
}
