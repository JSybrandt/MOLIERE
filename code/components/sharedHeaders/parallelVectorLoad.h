#include<list>
#include<map>
#include<string>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include "files.h"
using std::list;
using std::pair;
using std::string;
using std::fstream;
using std::stringstream;
using std::ios;
using std::unordered_map;

void string2vec(const string& line, string& label, vector<float>& vec){
  stringstream ss(line);
  ss >> label;
  float temp;
  while(ss >> temp){ vec.push_back(temp);}
}

// Appends elements from path to res
void fastLoadVecDir(const string& path, list<pair<string, vector<float>>>& res){
  vector<string> files = getFileOrFiles(path);
#pragma omp parallel for schedule(dynamic)
  for(unsigned int i = 0; i < files.size(); ++i){
    list<pair<string, vector<float>>> localVecs;
    string localPath = files[i];
    fstream localIn(localPath, ios::in);
    string line;
    while(getline(localIn, line)){
      string label;
      vector<float> vec;
      string2vec(line, label, vec);
      localVecs.emplace_back(label, vec);
    }
    localIn.close();
#pragma omp critical
    res.splice(res.end(), localVecs);
  }
}

// Appends elements from path to res
void fastLoadVecFile(const string& path, unordered_map<string, vector<float>>& res){
#pragma omp parallel
  {
#pragma omp single
  {
    fstream fin(path, ios::in);
    string line;
    while(getline(fin, line)){
#pragma omp task firstprivate(line)
      {
        vector<float> pt;
        string label;
        string2vec(line, label, pt);
#pragma omp critical
        res[label] = pt;
      }
    }
  }
  }
}
