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
#include"graphWithVectorInfo.h"
#include"parallelEdgeListLoad.h"
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

void string2vec(const string& line, string& label, vector<float>& vec){
  stringstream ss(line);
  ss >> label;
  float temp;
  while(ss >> temp){ vec.push_back(temp);}
}

vector<float> getVector(string ngrams, string pmid, string umls, string label){
  string targetFile = ngrams;
  if(label[0] == 'C')
    targetFile = umls;
  if(label[0] == 'P')
    targetFile = pmid;
  fstream fin(targetFile, ios::in);
  string line, token;
  vector<float> res;
  while(getline(fin, line)){
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
  cerr << "Failed to find " << label << " in " << targetFile << endl;
  throw getVectorException();
}

bool isInElipse(const vector<float>& fociiA,
                const vector<float>& fociiB,
                float elipseConst,
                const vector<float>& point){
  float d1 = distL2(point, fociiA);
  float d2 = distL2(point, fociiB);
  return d1 + d2 < elipseConst;
}

int main (int argc, char** argv){

  if(NUM_BYTE_PER_EDGE != 2 * sizeof(nodeIdx) + sizeof(float)){
    cout << "ERROR: the system does not have a 4 byte uint and a 4 byte float." << endl;
    throw invalidHardware();
  }

  cmdline::parser p;

  p.add<string>("graphFile", 'g', "input graph file", true);
  p.add<string>("sourceLbl", 's', "id representing the source", true);
  p.add<string>("targetLbl", 't', "intended target", true);
  p.add<string>("outputFile", 'o', "Output paths and neighborhoods", true);
  p.add<string>("ngramVectors", 'V',  "File contanining text vectors for ngrams", true);
  p.add<string>("pmidCentroids", 'P', "File containing text vectors for PMIDs", false, "");
  p.add<string>("umlsCentroids", 'U', "File containing text vectors for UMLS terms", true);
  p.add<string>("labelFile", 'l', "Label file accompanying the edges file.", true);
  p.add<float>("elipseConst", 'e', "Constant alpha where distL2(A,B)*\\alpha = 2a", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graphFile");
  string sourceLbl =  p.get<string>("sourceLbl");
  string targetLbl =  p.get<string>("targetLbl");
  string outputPath =  p.get<string>("outputFile");
  string vectorPath = p.get<string>("ngramVectors");
  string pmidCentroidPath = p.get<string>("pmidCentroids");
  string umlsCentroidPath = p.get<string>("umlsCentroids");
  string labelPath =  p.get<string>("labelFile");
  float elipseConstMultiple = p.get<float>("elipseConst");
  verbose = p.exist("verbose");

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
    sourceVec = getVector(vectorPath, pmidCentroidPath, umlsCentroidPath,  sourceLbl);

#pragma omp task
    targetVec = getVector(vectorPath, pmidCentroidPath, umlsCentroidPath, targetLbl);

#pragma omp taskwait
    elipseConst = distL2(sourceVec, targetVec) * elipseConstMultiple;

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
    fstream cFile(pmidCentroidPath, ios::in);
    fstream uFile(umlsCentroidPath, ios::in);
    string line;
    bool ignoreFirstLine = true;
    //get lines from both vectors and centroids
    while(getline(vFile, line) || getline(cFile, line) || getline(uFile, line)){
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
            if(verbose) cout << "In Ellipse: " << label << " : " << idx << endl;
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

  list<edge> edges;

  unordered_set<nodeIdx> keys(subsetVectors.size());
  for(const auto & pair : subsetVectors){
    keys.insert(pair.first);
  }
  fastLoadEdgeList(graphPath, edges, keys);

  graph g(subsetVectors, edges);

  if(::verbose) cout << "Loaded Graph with " << g.numNodes() << " nodes and "
                     << g.numEdges() << " edges."<< endl;

  if(::verbose) cout << "Checking if path exists:" << endl;

  vector<nodeIdx> path;

  if(::verbose) cout << "Getting shortest path." << endl;

  path = g.getShortestPath(label2idx[sourceLbl], label2idx[targetLbl]);

  if(::verbose){
    cout << "Path:";
    for(nodeIdx n : path)
      cout << n << " ";
    cout << endl;
  }

  fstream outFile(outputPath, ios::out);
  for(nodeIdx n : path)
    outFile << n << " ";
  outFile.close();

  return 0;
}
