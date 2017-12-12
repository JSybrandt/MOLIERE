
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
  p.add<unsigned int>("cloudSetN", 'N', "abstract cloud param: number of new abstracts adjacent to those on path.", false, 2000);
  p.add<unsigned int>("cloudSetC", 'C', "abstract cloud param: number of new abstracts from keyword overlap", false, 500);
  p.add<unsigned int>("cloudSetK", 'K', "abstract cloud param: number of new abstracts from keywords", false, 500);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graphFile");
  string pathPath =  p.get<string>("pathFile");
  string outputPath =  p.get<string>("outputFile");
  string labelPath =  p.get<string>("labelFile");
  unsigned int cloudSetN = p.get<unsigned int>("cloudSetN");
  unsigned int cloudSetC = p.get<unsigned int>("cloudSetC");
  unsigned int cloudSetK = p.get<unsigned int>("cloudSetK");
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
  vout << "Loading set of abstract nodeidx from " << labelPath << endl;
  graph g(labelPath);

  list<edge> edges;

  vout << "Loading first-order neighbors from " << graphPath << endl;
  fastLoadEdgeList(graphPath, edges, allNodes);

  for(const edge& e : edges){
    allNodes.insert(e.a);
    allNodes.insert(e.b);
  }

  edges.clear();

  vout << "Loading first and second order neighbors from " << graphPath << endl;
  fastLoadEdgeList(graphPath, edges, allNodes);

  vout << "constructing graph" << endl;
  for(const edge& e : edges){
    g.addEdge(e);
  }

  edges.clear();

  vout << "Found " << g.numNodes() << " nodes" << endl;
  vout << "Found " << g.numEdges() << " edges" << endl;


  fstream outFile(outputPath, ios::out);

  vout << "Getting cloud" << endl;
  unordered_set<nodeIdx> neighborhood = g.getCloud(path, cloudSetN, cloudSetC, cloudSetK);

  vout << "Writing to file" << endl;
  for(nodeIdx n : neighborhood)
    outFile << n << " ";
  outFile << endl;

  outFile.close();
  return 0;
}
