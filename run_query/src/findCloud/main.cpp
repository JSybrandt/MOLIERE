
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

  p.add<string>("graph", 'g', "input graph (BINARY) file", true);
  p.add<string>("path", 'p', "input path file (idx)", true);
  p.add<string>("output", 'o', "Output paths and neighborhoods", true);
  p.add<string>("labels", 'l', "Label file accompanying the edges file.", true);
  p.add<unsigned int>("abstracts-per-node", 'A', "The cloud will expand until it find this many abstracts per node in path", false, 3000);
  p.add<unsigned int>("max-dist", 'n', "max separation from path (hops)", false, 4);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graph");
  string pathPath =  p.get<string>("path");
  string outputPath =  p.get<string>("output");
  string labelPath =  p.get<string>("labels");
  unsigned int numAbstractsPerNode = p.get<unsigned int>("abstracts-per-node");
  unsigned int maxDistFromPath = p.get<unsigned int>("max-dist");
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

  list<edge> edgeList;
  unsigned int cycleCount = 0;
  bool needMoreNeighbors = false;

  cycleCount = loadAnotherOrderNeighbors(graphPath, edgeList, allNodes);

  do{

    edgeList.clear();
    vout << "Loading " << cycleCount + 1 << "-order neighbors" << endl;
    cycleCount = loadAnotherOrderNeighbors(graphPath, edgeList, allNodes);

    vout << "List 2 vec" << endl;
    vector<edge> edgeVec;
    edgeVec.reserve(edgeList.size());
    edgeVec.insert(edgeVec.end(),
        make_move_iterator(edgeList.begin()),
        make_move_iterator(edgeList.end())
    );

    vout << "constructing graph" << endl;
    unordered_map<nodeIdx, unordered_map<nodeIdx, float>> graphData;
    #pragma omp parallel
    {
      unordered_map<nodeIdx, unordered_map<nodeIdx, float>> localGraphData;
      #pragma omp for
      for(size_t i = 0; i < edgeVec.size(); ++i){
        const edge& e = edgeVec[i];
        localGraphData[e.a][e.b] = e.weight;
        localGraphData[e.b][e.a] = e.weight;
      }
      #pragma omp critical
      {
        for(auto& pair : localGraphData){
          graphData[pair.first].insert(
            make_move_iterator(pair.second.begin()),
            make_move_iterator(pair.second.end())
          );
        }
      }
    }

    g.setData(move(graphData));

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
