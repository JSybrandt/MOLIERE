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
#include"parallelAbstractLoad.h"
#include"parallelVectorLoad.h"
#include"util.h"
#include"graph.h"

#include<omp.h>

using namespace std;

bool verbose = false;
#define vout if(::verbose) cout

bool isInElipse(const vector<float>& fociiA,
                const vector<float>& fociiB,
                float elipseConst,
                const vector<float>& point){
  float d1 = distL2(point, fociiA);
  float d2 = distL2(point, fociiB);
  return d1 + d2 < elipseConst;
}

void fastLoadVecsFromEllipse(const string& vecPath,
                             list<pair<string, vector<float>>>& result,
                             const vector<float>& fociiA,
                             const vector<float>& fociiB,
                             float ellipseConst){

  vout << "Loading vecs from " << vecPath << endl;

  //get properties of abstract path
  struct stat st;
  stat(vecPath.c_str(), &st);
  size_t totalFileSize = st.st_size;

  vector<size_t> fileStarts;

#pragma omp parallel
  {
    unsigned int tid = omp_get_thread_num();
    unsigned int totalThreadNum = omp_get_num_threads();
    size_t bytesPerThread = totalFileSize / totalThreadNum;

#pragma omp single
    {
      fileStarts = vector<size_t>(totalThreadNum + 1, 0);
      fileStarts[totalThreadNum] = totalFileSize;
    }

#pragma omp barrier

    // each thread puts its start position
    fstream localVecFile(vecPath, ios::in | ios::binary);
    localVecFile.seekg(tid * bytesPerThread);

    string localLine;
    if(tid > 0){
      // jump to next newline
      getline(localVecFile, localLine);
    }

    fileStarts[tid] = localVecFile.tellg();

#pragma omp barrier

    list<pair<string, vector<float>>> localEllipse;

    // while we are still inside our own section
    while(localVecFile.tellg() < fileStarts[tid+1] && localVecFile){

      bool firstLineCheck = false;
      if(localVecFile.tellg() == fileStarts[0]){
        firstLineCheck = true;
      }

      getline(localVecFile, localLine);

      string lbl;
      if(firstLineCheck){
        stringstream ss (localLine);
        int count = 0;
        while(ss >> lbl) ++ count;
        //skip fasttext header at top of file
        vout << "Caught first line count of " << count << endl;
        if(count == 2) continue;
      } else {
      }

      vector<float> vec;
      string2vec(localLine, lbl, vec);
      if(stopwords.find(lbl) == stopwords.end() &&
         isInElipse(fociiA, fociiB, ellipseConst, vec))
      {
        localEllipse.emplace_back(lbl, vec);
      }
    }

    localVecFile.close();

#pragma omp critical
    {
      result.splice(result.end(), localEllipse);
    }

  }

}




int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("graphFile", 'g', "input graph file", true);
  p.add<string>("sourceLbl", 's', "id representing the source", true);
  p.add<string>("targetLbl", 't', "intended target", true);
  p.add<string>("outputFile", 'o', "Output paths and neighborhoods", true);
  p.add<string>("ngramVectors", 'V',  "File contanining text vectors for ngrams", true);
  p.add<string>("pmidCentroids", 'P', "File containing text vectors for PMIDs", true);
  p.add<string>("umlsCentroids", 'U', "File containing text vectors for UMLS terms", true);
  p.add<string>("labelFile", 'l', "Label file accompanying the edges file.", true);
  p.add<float>("elipseConst", 'e', "Constant alpha where distL2(A,B)*\\alpha = 2a", false, 1.4);
  p.add("pmidInEllipse", 'x', "Includes pmid vectors in the initial vector set.");
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graphFile");
  string sourceLbl =  p.get<string>("sourceLbl");
  string targetLbl =  p.get<string>("targetLbl");
  string outputPath =  p.get<string>("outputFile");
  string nGramVecPath = p.get<string>("ngramVectors");
  string pmidCentroidPath = p.get<string>("pmidCentroids");
  string umlsCentroidPath = p.get<string>("umlsCentroids");
  string labelPath =  p.get<string>("labelFile");
  float elipseConstMultiple = p.get<float>("elipseConst");
  verbose = p.exist("verbose");
  bool pmidInEllipse = p.exist("pmidInEllipse");

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

  unordered_set<string> vectorSubset = {sourceLbl, targetLbl};

  vout << "Loading inital vectors for ellipse" << endl;
  list<pair<string, vector<float>>> word2vecList;
  fastLoadVecs(nGramVecPath,
               pmidCentroidPath,
               umlsCentroidPath,
               word2vecList,
               vectorSubset);
  unordered_map<string, vector<float>> word2vec = {
    word2vecList.begin(),
    word2vecList.end()
  };

  vout << "Found " << word2vec.at(sourceLbl).size() << endl;
  vout << "Found " << word2vec.at(targetLbl).size() << endl;


  float elipseConst = distL2(word2vec.at(sourceLbl), word2vec.at(targetLbl)) * elipseConstMultiple;

  // we don't load papers, takes too long
  vector<string> filePaths = {nGramVecPath, umlsCentroidPath};
  if(pmidInEllipse) filePaths.push_back(pmidCentroidPath);

  for(const string& path : filePaths){
    vout << "Loading nearby vectors from " << path << endl;
    fastLoadVecsFromEllipse(path,
                            word2vecList,
                            word2vec[sourceLbl],
                            word2vec[targetLbl],
                            elipseConst);
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
