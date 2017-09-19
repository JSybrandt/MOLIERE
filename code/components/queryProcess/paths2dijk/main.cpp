/*
 * THIS PROGRAM ASSUMES WE HAVE USED A DIFFERENT ALG TO FIND THE PATHS
 * AND WE TAKE THAT INFO TO GENERATE THE DIJK FILE
 */

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

#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"cmdln.h"
#include"pQueue.h"
#include"graph.h"
#include"util.h"


using namespace std;

bool verbose = false;

class invalidBinaryFile : public exception{};

void loadBinGraph(string graphPath, graph& g){

  if(::verbose) cout << "Loading Graph." << endl;

  struct stat st;
  stat(graphPath.c_str(), &st);
  size_t totalFileSize = st.st_size;
  if(totalFileSize % NUM_BYTE_PER_EDGE){
    cerr << "Graph has " << totalFileSize << " bytes, which is not a multiple of "
         << NUM_BYTE_PER_EDGE << endl;
    throw invalidBinaryFile();
  }
  size_t numEdges = totalFileSize / NUM_BYTE_PER_EDGE;

  if(::verbose){
    cout << "Number of Edges:" << numEdges << endl;
  }

#pragma omp parallel
  {

    unsigned int tid = omp_get_thread_num();
    unsigned int totalThreadNum = omp_get_num_threads();
    size_t edgesPerThread = numEdges / totalThreadNum;

    size_t startEdgeNum = tid * edgesPerThread;
    size_t endEdgeNum = startEdgeNum + edgesPerThread;
    if(tid == totalThreadNum-1){
      endEdgeNum = numEdges;
    }

    vector<edge> localEdges;

    if(::verbose){
#pragma omp critical
      cout << tid << ":\t" << startEdgeNum << " - " << endEdgeNum << endl;
#pragma omp barrier
    }

    FILE* file;
    file = fopen(graphPath.c_str(), "rb");
    fseek(file, startEdgeNum * NUM_BYTE_PER_EDGE, SEEK_SET);
    charBuff buff[3];
    while(startEdgeNum < endEdgeNum){
      fread(buff, sizeof(charBuff), 3, file);
      edge e(buff[0].i, buff[1].i, buff[2].f);
      localEdges.push_back(e);
    }
#pragma omp critical
    {
      for(const edge& e : localEdges)
        g.addEdge(e);
    }
  }
}

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

  // CONSTRUCTING GRAPH
  graph g(labelPath);
  loadBinGraph(graphPath, g);

  if(verbose) cout << "Found " << g.numNodes() << " nodes" << endl;
  if(verbose) cout << "Found " << g.numEdges() << " edges" << endl;


  vector<vector<nodeIdx>> paths;
  fstream pathFile(pathPath, ios::in);
  string line;
  nodeIdx node;
  while(getline(pathFile, line)){
    stringstream ss(line);
    vector<nodeIdx> path;
    while(ss >> node)
      path.push_back(node);
    paths.push_back(path);
  }
  pathFile.close();

  // OUTPUT PATHS
  fstream outFile(outputPath, ios::out);

#pragma omp parallel for schedule(dynamic)
  for(unsigned int i = 0; i < paths.size(); ++i){
    const vector<nodeIdx>& p = paths[i];
    if(p.size() > 1){
      unordered_set<nodeIdx> neighborhood = g.getCloud(p, cloudSetN, cloudSetC, cloudSetK);
#pragma omp critical
      {
        for(nodeIdx n : p)
          outFile << n << " ";
        outFile << endl;
        for(nodeIdx n : neighborhood)
          outFile << n << " ";
        outFile << endl;
      }
    }
  }
  outFile.close();
  return 0;
}
