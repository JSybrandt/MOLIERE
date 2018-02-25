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
#include"pQueue.h"
#include"graph.h"
#include"util.h"
#include"parallelEdgeListLoad.h"

bool verbose = false;
#define vout if(::verbose) cout

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("graphFile", 'g', "input graph (BINARY) file", true);
  p.add<string>("labelFile", 'l', "Label file accompanying the edges file.", true);
  p.add("verbose", 'v', "outputs debug information");
  p.add<string>("target", 't', "label of the important node");
  p.add<unsigned int>("numNeighbors", 'n', "number of nearby nodes");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graphFile");
  string labelPath =  p.get<string>("labelFile");
  unsigned int numNeighbors = p.get<unsigned int>("numNeighbors");
  string targetLbl =  p.get<string>("target");
  verbose = p.exist("verbose");

  nodeIdx targetIdx = UNDEFINED;

  // CONSTRUCTING GRAPH
  vout << "Loading labels from " << labelPath << endl;
  vector<string> labels;
  fstream lFile(labelPath, ios::in);
  string tmp;
  while(lFile >> tmp){
    if(tmp == targetLbl) targetIdx = labels.size();
    labels.push_back(tmp);
  }
  lFile.close();


  list<edge> edges;
  unordered_set<nodeIdx> allNodes;
  allNodes.insert(targetIdx);

  unsigned int count = 0;
  while(allNodes.size() < numNeighbors + 1){
    count += 1;
    vout << "Loading " << count << " neighbors" << endl;
    fastLoadEdgeList(graphPath, edges, allNodes);
    for(edge& e : edges){
      allNodes.insert(e.a);
      allNodes.insert(e.b);
    }
  }

  Graph g;
  for(edge &e : edges){
    g.addEdge(e.a, e.b, e.weight);
  }

  vout << "Getting neighbors" << endl;
  unordered_set<nodeIdx> neighbors = g.getNeighbors(targetIdx, numNeighbors);

  for(nodeIdx idx : neighbors){
    cout << labels[idx] << endl;
  }








}
