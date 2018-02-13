#include<list>
#include<map>
#include<string>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include<unordered_set>
#include"files.h"

// needed for posix io
#include<cstdio>
#include<sys/types.h>
#include<sys/stat.h>
#include<omp.h>

using std::list;
using std::pair;
using std::string;
using std::fstream;
using std::stringstream;
using std::ios;
using std::unordered_map;
using std::unordered_set;

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


void fastLoadVecSubset(const string& vecPath,
                       list<pair<string, vector<float>>>& result,
                       const unordered_set<string>& subset = unordered_set<string>()){

  bool takeAll = (subset.size() == 0);

  //get properties of abstract path
  struct stat st;
  stat(vecPath.c_str(), &st);
  size_t totalFileSize = st.st_size;

  vector<size_t> fileStarts;

  unsigned int totalLines;

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

    list<pair<string, vector<float>>> nGram2Vec;

    // while we are still inside our own section
    unsigned int numLines = 0;
    while(localVecFile.tellg() < fileStarts[tid+1] && localVecFile){
      getline(localVecFile, localLine);
      numLines += 1;

      unsigned int space = localLine.find(' ');
      string nGram = localLine.substr(0, space);
      if(takeAll || subset.find(nGram) != subset.end()){
        vector<float> vec;
        string2vec(localLine, nGram, vec);
        nGram2Vec.emplace_back(nGram, vec);
      }
    }

    localVecFile.close();

#pragma omp critical
    {
      result.splice(result.end(), nGram2Vec);
    }

  }

}
