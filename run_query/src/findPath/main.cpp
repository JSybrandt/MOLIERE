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
#include<cmath>
#include<cstring>
#include<exception>

// needed for posix io
#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"cmdln.h"
#include"parallelEdgeListLoad.h"
#include"parallelFileOp.h"
#include"util.h"
#include"graph.h"
#include"labelManager.h"

#include<omp.h>

using namespace std;

bool verbose = false;
#define vout if(::verbose) cout

bool isInEllipse(const vector<float>& fociiA,
                const vector<float>& fociiB,
                float elipseConst,
                const vector<float>& point){
  float d1 = distL2(point, fociiA);
  float d2 = distL2(point, fociiB);
  return d1 + d2 < elipseConst;
}

vector<float> vecA, vecC;
float ellipseConst;

bool selectVecFromEllipse(const string& line){
  const pair<string, vector<float>>& l2v = line2vec(line);
  return isInEllipse(vecA, vecC, ellipseConst, l2v.second);
}

unordered_set<string> vectorSubsetLabels;
bool selectVecFromSubset(const string& line){
  stringstream ss(line);
  string label;
  ss >> label;
  return (vectorSubsetLabels.find(label) != vectorSubsetLabels.end());
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("graph", 'g', "input graph file", true);
  p.add<string>("source-label", 's', "id representing the source", true);
  p.add<string>("target-label", 't', "intended target", true);
  p.add<string>("output", 'o', "Output paths and neighborhoods", true);
  p.add<string>("labels", 'l', "Label file accompanying the edges file.", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graph");
  string sourceLbl =  p.get<string>("source-label");
  string targetLbl =  p.get<string>("target-label");
  string outputPath =  p.get<string>("output");
  string labelPath =  p.get<string>("labels");
  verbose = p.exist("verbose");

  vout << "Loading Labels:" << endl;
  LabelManager labels(labelPath);

  unordered_set<nodeIdx> allNodes = {
    labels[sourceLbl], labels[targetLbl]
  };

  vout << "Making Graph from #nodes: " << allNodes.size() << endl;
  Graph graph(graphPath, allNodes);

  vout << "Loaded Graph with " << graph.numNodes() << " nodes and "
                     << graph.numEdges() << " edges."<< endl;

  vout << "Checking if path exists:" << endl;
  vector<nodeIdx> path = graph.getShortestPath(labels[sourceLbl],
                                               labels[targetLbl]);
  while(path.size() == 0){
    allNodes = graph.getNodes();
    vout << "Rebuilding Graph" << endl;
    graph = Graph(graphPath, allNodes);
    path = graph.getShortestPath(labels[sourceLbl], labels[targetLbl]);
  }

  if(::verbose){
    cout << "Path:";
    for(nodeIdx n : path)
      cout << labels[n] << " ";
    cout << endl;
  }

  fstream outFile(outputPath, ios::out);
  for(nodeIdx n : path)
    outFile << n << " ";
  outFile.close();

  return 0;
}
