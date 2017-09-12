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

#include"pQueue.h"
#include"cmdln.h"

#include<omp.h>

using namespace std;

bool verbose = false;

// TYPES
typedef unsigned int nodeIdx;
typedef unordered_map<nodeIdx, vector<float>> VecDict;
const nodeIdx UNDEFINED = numeric_limits<nodeIdx>::max();

class getLabelException: public exception {};

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

void string2vec(string line, string& label, vector<float>& vec){
  stringstream ss(line);
  ss >> label;
  float temp;
  while(ss >> temp){ vec.push_back(temp);}
}

class getVectorException: public exception {};

vector<float> getVector(string path, string label){
  fstream fin(path, ios::in);
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
  throw getVectorException();
}

class vectorLengthNotEqual: public exception {};

float dist(const vector<float>& ptA, const vector<float>& ptB){
  if(ptA.size() != ptB.size()){
    cerr << ptA.size() << " != " << ptB.size() << endl;
    throw vectorLengthNotEqual();
  }

  float res = 0;
  for(unsigned int i = 0; i < ptA.size(); ++i){
    res += pow(ptA[i]-ptB[i], 2);
  }
  return sqrt(res);
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
      cout << sourceIdx << " ->" << sourceLbl << endl;
      cout << targetIdx << " ->" << targetLbl << endl;
    }

#pragma omp task
    sourceVec = getVector(vectorPath, sourceLbl);

#pragma omp task
    targetVec = getVector(vectorPath, targetLbl);

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
          nodeIdx idx = getIdx(labelPath, label);
#pragma omp critical
          {
            subsetVectors[idx] = pt;
            if(verbose) cout << "In Elipse: " << label << endl;
          }
        }
      }
    }
    vFile.close();
    cFile.close();

  }
  }

  return 0;
}
