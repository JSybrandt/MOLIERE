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
#include"graph.h"
#include"util.h"

#include<omp.h>

using namespace std;

bool verbose = false;


class invalidBinaryFile: public exception {};
class getLabelException: public exception {};
class invalidHardware: public exception {};

string getLabel(string path, nodeIdx idx){
  nodeIdx count = 0;
  fstream fin(path, ios::in);
  string token;
  while(fin >> token){
    if(count == idx){
      return token;
    }
    ++count;
  }
  throw getLabelException();
}

class getIdxException: public exception {};

nodeIdx getIdx(string path, string label){
  nodeIdx count = 0;
  fstream fin(path, ios::in);
  string token;
  while(fin >> token){
    if(token == label){
      return count;
    }
    ++count;
  }
  throw getIdxException();
}


class getVectorException: public exception {};

vector<float> getVector(string path1, string path2, string label){
  fstream fin1(path1, ios::in);
  fstream fin2(path2, ios::in);
  string line, token;
  vector<float> res;
  while(getline(fin1, line) || getline(fin2, line)){
    // line starts with label, not sufficient, but good filter
    if(strncmp(line.c_str(), label.c_str(), label.length()) == 0){
      stringstream ss(line);
      ss >> token;
      if(token == label){
        string2vec(line, label, res);
        return res;
      }
    }
  }
  cerr << "Failed to find " << label << " in " << path1 << " or " << path2 << endl;
  throw getVectorException();
}

bool isInElipse(const vector<float>& fociiA,
                const vector<float>& fociiB,
                float elipseConst,
                const vector<float>& point){
  float d1 = dist(point, fociiA);
  float d2 = dist(point, fociiB);
  return d1 + d2 < elipseConst;
}

int main (int argc, char** argv){

  if(NUM_BYTE_PER_EDGE != 2 * sizeof(nodeIdx) + sizeof(float)){
    cout << "ERROR: the system does not have a 4 byte uint and a 4 byte float." << endl;
    throw invalidHardware();
  }

  cmdline::parser p;

  p.add<string>("graphFile", 'g', "input graph file", true);
  p.add<nodeIdx>("sourceIdx", 's', "id representing the source", true);
  p.add<nodeIdx>("targetIdx", 't', "intended target", false, UNDEFINED);
  p.add<string>("outputFile", 'o', "Output paths and neighborhoods", true);
  p.add<string>("vectorFile", 'V', "Dict File for vectors", true);
  p.add<string>("centroidFile", 'C', "Dict File for centroids", true);
  p.add<string>("labelFile", 'l', "Label file accompanying the edges file.", true);
  p.add<float>("elipseConst", 'e', "Constant alpha where dist(A,B)*\\alpha = 2a", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graphFile");
  nodeIdx sourceIdx =  p.get<nodeIdx>("sourceIdx");
  nodeIdx targetIdx =  p.get<nodeIdx>("targetIdx");
  string outputPath =  p.get<string>("outputFile");
  string vectorPath = p.get<string>("vectorFile");
  string centroidPath = p.get<string>("centroidFile");
  string labelPath =  p.get<string>("labelFile");
  float elipseConstMultiple = p.get<float>("elipseConst");
  verbose = p.exist("verbose");

  string sourceLbl, targetLbl;
  vector<float> sourceVec, targetVec;
  VecDict subsetVectors;
  float elipseConst;

  unordered_map<string, nodeIdx> label2idx;
  if(::verbose) cout << "Loading Labels:" << endl;
  nodeIdx count = 0;
  fstream fin(labelPath, ios::in);
  string token;
  while(fin >> token){
    label2idx[token] = count;
    ++count;
  }

#pragma omp parallel
  {
#pragma omp single
  {
#pragma omp task
    sourceLbl = getLabel(labelPath, sourceIdx);
#pragma omp task
    targetLbl = getLabel(labelPath, targetIdx);

#pragma omp taskwait
    if(verbose){
      cout << sourceIdx << " -> " << sourceLbl << endl;
      cout << targetIdx << " -> " << targetLbl << endl;
    }

#pragma omp task
    sourceVec = getVector(vectorPath, centroidPath,  sourceLbl);

#pragma omp task
    targetVec = getVector(vectorPath, centroidPath, targetLbl);

#pragma omp taskwait
    elipseConst = dist(sourceVec, targetVec) * elipseConstMultiple;

    if(verbose){
      cout << sourceLbl << "-> ";
      for(float f : sourceVec){
        cout << f << " ";
      }
      cout << endl;
      cout << targetLbl << "-> ";
      for(float f : targetVec){
        cout << f << " ";
      }
      cout << endl;
      cout << "Elipse Constant:" << elipseConst << endl;
    }

    fstream vFile(vectorPath, ios::in);
    fstream cFile(centroidPath, ios::in);
    string line;
    bool ignoreFirstLine = true;
    //get lines from both vectors and centroids
    while(getline(vFile, line) || getline(cFile, line)){
      if(ignoreFirstLine){ // vector file has size at start
        ignoreFirstLine = false;
        continue;
      }
#pragma omp task firstprivate(line)
      {
        vector<float> pt;
        string label;
        string2vec(line, label, pt);
        if(isInElipse(sourceVec, targetVec, elipseConst, pt)){
          nodeIdx idx = label2idx[label];
#pragma omp critical
          {
            subsetVectors[idx] = pt;
            if(verbose) cout << "In Elipse: " << label << " : " << idx << endl;
          }
        }
      }
    }
    vFile.close();
    cFile.close();

  }
  }

  if(::verbose) cout << "Found " << subsetVectors.size() << " subset items." << endl;
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
  vector<edge> edges;

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

      if(subsetVectors.find(e.a) != subsetVectors.end() ||
         subsetVectors.find(e.b) != subsetVectors.end())
      {
#pragma omp critical
        edges.push_back(e);
      }
      ++startEdgeNum;
    }
    fclose(file);
  }

  graph g(subsetVectors, edges);

  if(::verbose) cout << "Loaded Graph with " << g.numNodes() << " nodes and "
                     << g.numEdges() << " edges."<< endl;

  if(::verbose) cout << "Checking if path exists:" << endl;

  vector<nodeIdx> path;
  if(g.pathExists(sourceIdx, targetIdx)){

    if(::verbose) cout << "Getting shortest path." << endl;

    path = g.getShortestPath(sourceIdx, targetIdx);

    if(::verbose){
      cout << "Path:";
      for(nodeIdx n : path)
        cout << n << " ";
      cout << endl;
    }
  } else {
    if(::verbose) cout << "Path does not exist. Consider extending elipse." << endl;
  }

  fstream outFile(outputPath, ios::out);
  for(nodeIdx n : path)
    outFile << n << " ";
  outFile.close();

  return 0;
}
