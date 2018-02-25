
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

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;



int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("graphFile", 'g', "input graph (BINARY) file", true);
  p.add<string>("pathFile", 'p', "input path file (idx)", true);
  p.add<string>("outputFile", 'o', "Output paths and neighborhoods", true);
  p.add<string>("labelFile", 'l', "Label file accompanying the edges file.", true);
  p.add<unsigned int>("numAbstractsPerNode", 'A', "The cloud will expand until it find this many abstracts per node in path", false, 3000);
  p.add<unsigned int>("maxDistFromPath", 'n', "max separation from path (hops)", false, 4);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graphFile");
  string pathPath =  p.get<string>("pathFile");
  string outputPath =  p.get<string>("outputFile");
  string labelPath =  p.get<string>("labelFile");
  unsigned int numAbstractsPerNode = p.get<unsigned int>("numAbstractsPerNode");
  unsigned int maxDistFromPath = p.get<unsigned int>("maxDistFromPath");
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

  unsigned int maxResult = numAbstractsPerNode * path.size();
  vout << "Expecting a min resulting cloud of size " << maxResult << endl;

  // CONSTRUCTING GRAPH
  vout << "Loading labels from " << labelPath << endl;
  graph g(labelPath);
  unordered_set<nodeIdx> cloud;

  list<edge> edges;
  unsigned int cycleCount = 0;
  bool needMoreNeighbors = false;

  cycleCount = loadAnotherOrderNeighbors(graphPath, edges, allNodes);

  do{

    edges.clear();
    vout << "Loading " << cycleCount + 1 << "-order neighbors" << endl;
    cycleCount = loadAnotherOrderNeighbors(graphPath, edges, allNodes);

    vout << "constructing graph" << endl;
    for(const auto & e : edges)
      g.addEdge(e);

    vout << "Found " << g.numNodes() << " nodes" << endl;
    vout << "Found " << g.numEdges() << " edges" << endl;

    vout << "Getting cloud" << endl;
    cloud.clear();
    needMoreNeighbors = false;
#pragma omp parallel for schedule(dynamic)
    for(unsigned int i = 0; i < path.size(); ++i){
      unordered_set<nodeIdx> local = g.getCloud(path[i], numAbstractsPerNode);
      vout << "Found " << local.size() << " near " << path[i] << endl;
#pragma omp critical
      {
        if(local.size() < numAbstractsPerNode)
          needMoreNeighbors = true;
        cloud.insert(local.begin(), local.end());
      }
    }

    vout << "Found cloud of size " << cloud.size() << endl;

  }while(needMoreNeighbors && cycleCount < maxDistFromPath);

  fstream outFile(outputPath, ios::out);

  vout << "Writing to file" << endl;
  for(nodeIdx n : cloud)
    outFile << n << endl;

  outFile.close();
  return 0;
}
