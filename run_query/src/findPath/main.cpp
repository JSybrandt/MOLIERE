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
  p.add<string>("ngram-vecs", 'N',  "File contanining text vectors for ngrams", true);
  p.add<string>("pmid-vecs", 'P', "File containing text vectors for PMIDs", true);
  p.add<string>("umls-vecs", 'U', "File containing text vectors for UMLS terms", true);
  p.add<string>("labels", 'l', "Label file accompanying the edges file.", true);
  p.add<float>("ellipse-const", 'e', "Constant alpha where distL2(A,B)*\\alpha = 2a", false, 1.4);
  p.add("include-pmid", 'x', "Includes pmid vectors in the initial vector set.");
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graph");
  string sourceLbl =  p.get<string>("source-label");
  string targetLbl =  p.get<string>("target-label");
  string outputPath =  p.get<string>("output");
  string ngramVecPath = p.get<string>("ngram-vecs");
  string pmidVecPath = p.get<string>("pmid-vecs");
  string umlsVecPath = p.get<string>("umls-vecs");
  string labelPath =  p.get<string>("labels");
  float elipseConstMultiple = p.get<float>("ellipse-const");
  verbose = p.exist("verbose");
  bool pmidInEllipse = p.exist("include-pmid");

  unordered_map<string, nodeIdx> label2idx;
  vector<string> labels;
  vout << "Loading Labels:" << endl;
  nodeIdx count = 0;
  fstream fin(labelPath, ios::in);
  string token;
  while(fin >> token){
    label2idx[token] = count;
    labels.push_back(token);
    ++count;
  }

  vout << "Loading inital vectors for ellipse" << endl;
  vectorSubsetLabels.insert(sourceLbl);
  vectorSubsetLabels.insert(targetLbl);
  vector<string> paths;
  if(sourceLbl[0] == 'P' || targetLbl[0] == 'P')
    paths.push_back(pmidVecPath);
  if(sourceLbl[0] == 'C' || targetLbl[0] == 'C')
    paths.push_back(umlsVecPath);
  if(sourceLbl[0] != 'P' || sourceLbl[0] != 'C' ||
     targetLbl[0] != 'P' || targetLbl[0] != 'C')
    paths.push_back(ngramVecPath);

  list<pair<string, vector<float>>> word2vecList;
  for(const string& path : paths)
    fastProcFile<pair<string, vector<float>>>(
        path, word2vecList, line2vec, selectVecFromSubset);
  unordered_map<string, vector<float>> word2vec {
    word2vecList.begin(),
    word2vecList.end()
  };

  vecA = word2vec.at(sourceLbl);
  vecC = word2vec.at(targetLbl);

  vout << "Found " << word2vec.at(sourceLbl).size() << endl;
  vout << "Found " << word2vec.at(targetLbl).size() << endl;


  ellipseConst = distL2(vecA, vecC) * elipseConstMultiple;

  // we don't load papers, takes too long
  paths = {ngramVecPath, umlsVecPath};
  if(pmidInEllipse) paths.push_back(pmidVecPath);

  for(const string& path : paths){
    vout << "Loading nearby vectors from " << path << endl;
    fastProcFile<pair<string, vector<float>>>(
        path, word2vecList, line2vec, selectVecFromEllipse);
  }

  if(::verbose){
    cout << "found:" << endl;
    for(const auto & p : word2vecList) cout << p.first << endl;
  }

  vout << "Loading Graph." << endl;

  list<edge> edges;

  unordered_set<nodeIdx> allNodes;
  for(const auto & pair : word2vecList){
    if(label2idx.find(pair.first) != label2idx.end()){
      allNodes.insert(label2idx.at(pair.first));
    } else {
      vout << "Failed to find vector for:" << pair.first << endl;
    }
  }

  fastLoadEdgeList(graphPath, edges, allNodes);

  Graph graph;
  for(edge e : edges){
    graph.addEdge(e.a, e.b, e.weight);
  }

  vout << "Loaded Graph with " << graph.numNodes() << " nodes and "
                     << graph.numEdges() << " edges."<< endl;

  vout << "Checking if path exists:" << endl;

  vout << "Getting shortest path." << endl;


  Graph::Path path = graph.getShortestPath(label2idx[sourceLbl], label2idx[targetLbl]);
  unsigned int retryCount = 0;
  while(path.path.size() == 0 && retryCount < 5){
    retryCount += 1;
    vout << "FAILED, restarting: " << retryCount << endl;
    edges.clear();
    vout << "Loading..." << endl;
    loadAnotherOrderNeighbors(graphPath, edges, allNodes);
    vout << "Building..." << endl;
    graph.addEdges(edges);
    vout << "Finding Path" << endl;
    path = graph.getShortestPath(label2idx[sourceLbl], label2idx[targetLbl]);
  }

  if(path.path.size() == 0){
    throw runtime_error("Failed to connect words in subnetwork");
  }

  if(::verbose){
    cout << "Path:";
    for(nodeIdx n : path.path)
      cout << labels[n] << " ";
    cout << endl;
  }

  fstream outFile(outputPath, ios::out);
  for(nodeIdx n : path.path)
    outFile << n << " ";
  outFile.close();

  return 0;
}
