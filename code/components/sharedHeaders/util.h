#pragma once

#include<vector>
#include<iostream>
#include<sstream>
#include<exception>
#include<cmath>
#include<fstream>
#include<cstring>
#include<unordered_set>
#include<unordered_map>
#include<limits>


using std::vector;
using std::cerr;
using std::endl;
using std::stringstream;
using std::exception;
using std::string;
using std::fstream;
using std::ios;
using std::strcmp;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::numeric_limits;

class vectorLengthNotEqual: public exception {};

float distL2(const vector<float>& ptA, const vector<float>& ptB){
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

float cosSim(const vector<float>& ptA, const vector<float>& ptB){
  if(ptA.size() != ptB.size()){
    cerr << ptA.size() << " != " << ptB.size() << endl;
    throw vectorLengthNotEqual();
  }

  float aDotB = 0, normA = 0, normB = 0;
  for(unsigned int i = 0; i < ptA.size(); ++i){
    float a = ptA[i], b = ptB[i];
    aDotB += a * b;
    normA += a * a;
    normB += b * b;
  }
  return aDotB / (sqrt(normA) * sqrt(normB));
}

union charBuff{
  unsigned int i;
  float f;
  char buff[4];
};

void operator+= (vector<float>& a, const vector<float>& b){
  if(a.size() != b.size())
    throw vectorLengthNotEqual();
  for(unsigned int i = 0; i < a.size(); ++i){
    a[i] += b[i];
  }
}

vector<float> operator+ (const vector<float>& a, const vector<float>& b){
  if(a.size() != b.size())
    throw vectorLengthNotEqual();
  vector<float> res(a.size(), 0);
  for(unsigned int i = 0; i < a.size(); ++i){
    res[i] = a[i] + b[i];
  }
  return res;
}

void operator/= (vector<float>& a, float b){
  for(unsigned int i = 0; i < a.size(); ++i){
    a[i] /= b;
  }
}

void operator*= (vector<float>& a, float b){
  for(unsigned int i = 0; i < a.size(); ++i){
    a[i] *= b;
  }
}

vector<float> operator* (const vector<float>& a, float b){
  vector<float> res(a.size());
  for(unsigned int i = 0; i < a.size(); ++i){
    res[i] = a[i] * b;
  }
  return res;
}

float magnitude(const vector<float>& a){
  float sumsqrd = 0;
  for(float f : a){
    sumsqrd += f * f;
  }
  return sqrt(sumsqrd);
}

class getVectorException: public exception {};
class neverGaveMeAnyDamnFilesException: public exception {};

class GetVector{
public:
  GetVector(string ngramPath, string pmidPath = "", string umlsPath = ""):ngramPath(ngramPath), pmidPath(pmidPath), umlsPath(umlsPath){};

  unordered_map<string, vector<float>> operator() (const unordered_set<string>& labels) const {
    unordered_map<string, vector<float>> res;
    unordered_set<string> paths;
    for(const string& label : labels){
      if(label[0] == 'C')
        paths.insert(umlsPath);
      else if (label[0] == 'P')
        paths.insert(pmidPath);
      else
        paths.insert(ngramPath);
    }
    unsigned int vSize = getVecSize();
    for(const string& path : paths){
      fstream fin(path, ios::in);
      string line;
#pragma omp parallel
      {
#pragma omp single
      {
        while(getline(fin, line)){
#pragma omp task firstprivate(line)
          {
            string token;
            stringstream ss(line);
            ss >> token;
            if(labels.find(token) != labels.end()){
              vector<float> vec(vSize);
              string crap;
              float temp;
              unsigned int count = 0;
              stringstream ss(line);
              ss >> crap;
              while(ss >> temp){ vec[count] = temp; ++count; }
#pragma omp critical(res)
              res[token] = vec;
            }
          }
        }
      }
      }
    }

    //if(labels.size() > 0)
      //cerr << "warning, did not find all items" << endl;
    return res;
  }

  vector<float> operator() (const string & label) const {
    string targetFile = ngramPath;
    if(label[0] == 'C')
      targetFile = umlsPath;
    if(label[0] == 'P')
      targetFile = pmidPath;

    fstream fin(targetFile, ios::in);
    string line, token;
    vector<float> res(getVecSize());
    while(getline(fin, line)){
      // line starts with label, not sufficient, but good filter
      if(strncmp(line.c_str(), label.c_str(), label.length()) == 0){
        stringstream ss(line);
        ss >> token;
        if(token == label){
          string crap;
          float temp;
          unsigned int count = 0;
          stringstream ss(line);

          ss >> crap;
          while(ss >> temp){ res[count] = temp; ++count; }
          return res;
        }
      }
    }
    cerr << "Failed to find " << label << " in " << targetFile << endl;
    throw getVectorException();
  }

  unsigned int getVecSize() const {
    static unsigned int vecSize = 0;
    static bool init = false;
    if(! init){
      string p = ngramPath;
      if(p == "")
        p = pmidPath;
      if(p == "")
        p = umlsPath;
      if(p == "")
        throw neverGaveMeAnyDamnFilesException();
      fstream fin(p, ios::in);
      string line, label;
      getline(fin, line);
      getline(fin, line);
      unsigned int size = 0;
      stringstream ss(line);
      string token;
      while(ss >> token){ ++size; }
      vecSize = size - 1; // -1 becuase label was counted
    }
    return vecSize;
  }
private:
  string ngramPath, pmidPath, umlsPath;
};

template<class T, class K>
bool cmpRev(const pair<T,K>& a, const pair<T,K>& b){
  return a.first > b.first;
}


const unsigned int NUM_BYTE_PER_EDGE = 12;
typedef unsigned int nodeIdx;
const nodeIdx UNDEFINED = numeric_limits<nodeIdx>::max();
struct edge{
  edge(): a(UNDEFINED), b(UNDEFINED), weight(0) {}
  edge(nodeIdx a, nodeIdx b, float w): a(a), b(b), weight(w) {}
  nodeIdx a, b;
  float weight;
};

