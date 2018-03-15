#pragma once

#include<string>
#include<unordered_map>
#include<vector>
#include<list>
#include<fstream>
#include<sstream>
#include<iostream>

// needed for posix io
#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include<omp.h>

#include"util.h"

using std::string;
using std::unordered_map;
using std::pair;
using std::vector;
using std::list;
using std::fstream;
using std::ios;
using std::stringstream;

class LabelManager {
public:
  LabelManager(const string& lblPath){
    fastLoadLabels(lblPath);
  };

  nodeIdx operator[](const string& lbl) {
    cache(lbl);
    return lbl2idxCache[lbl];
  }

  string operator[](nodeIdx idx) const{
    return idx2lbl[idx];
  }

  bool contains(const string& lbl) {
    try{
      cache(lbl); // either works or throws exception
      return true;
    } catch(std::runtime_error){
      return false;
    }
  }

  bool contains(nodeIdx idx) const{
    return idx < idx2lbl.size();
  }

  size_t size() const {
    return idx2lbl.size();
  }

private:
  unordered_map<string, nodeIdx> lbl2idxCache;
  vector<string> idx2lbl;

  void cache(const string& lbl){
    if(lbl2idxCache.find(lbl) == lbl2idxCache.end()){
      bool found = false;
      #pragma omp parallel for shared(found)
      for(size_t i = 0; i < idx2lbl.size(); ++i){
        if(found) continue;
        if(idx2lbl[i] == lbl){
          lbl2idxCache[lbl] = i;
          found = true;
        }
      }
      if(!found){
        throw std::runtime_error("Failed to find " + lbl);
      }
    }
  }

  void fastLoadLabels(const string& filePath){

    //get properties of abstract path
    struct stat st;
    stat(filePath.c_str(), &st);
    size_t totalFileSize = st.st_size;

    vector<size_t> fileStarts;
    vector<size_t> labelStarts;

    #pragma omp parallel
    {
      unsigned int tid = omp_get_thread_num();
      unsigned int totalThreadNum = omp_get_num_threads();
      size_t bytesPerThread = totalFileSize / totalThreadNum;

      #pragma omp single
      {
        fileStarts = vector<size_t>(totalThreadNum + 1, 0);
        fileStarts[totalThreadNum] = totalFileSize;
        labelStarts = vector<size_t>(totalThreadNum+1, 0);
      }

      #pragma omp barrier

      // each thread puts its start position
      fstream localFile(filePath, ios::in | ios::binary);
      localFile.seekg(tid * bytesPerThread);

      string localLine;
      if(tid > 0){
        // jump to next newline
        getline(localFile, localLine);
      }

      fileStarts[tid] = localFile.tellg();

      #pragma omp barrier

      vector<string> localLabels;
      localLabels.reserve(fileStarts[tid+1] - fileStarts[tid] + 10);

      // while we are still inside our own section
      size_t numLines = 0;
      while(localFile.tellg() < fileStarts[tid+1] && localFile){
        getline(localFile, localLine);
        numLines += 1;
        localLabels.emplace_back(move(localLine));
      }

      localFile.close();

      labelStarts[tid+1] = localLabels.size();
      #pragma omp barrier

      #pragma omp single
      {
        // prefix sum
        for(size_t i = 1; i < labelStarts.size(); ++i){
          labelStarts[i] += labelStarts[i-1];
        }
        // create
        idx2lbl = vector<string>(labelStarts[totalThreadNum]);
      }

      #pragma omp barrier

      size_t start = labelStarts[tid];
      size_t end = labelStarts[tid+1];
      for(size_t i = start; i < end; ++i){
        idx2lbl[i] = move(localLabels[i-start]);
      }
    }
  }
};
