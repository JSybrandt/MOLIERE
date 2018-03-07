#pragma once

#include<omp.h>

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
#include<list>

// needed for posix io
#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"cmdln.h"
#include"util.h"

#include<omp.h>

using namespace std;



void fastLoadEdgeList(const string& graphPath, // file path
                      list<edge>& result,  //pass by ref result
                      const unordered_set<nodeIdx>& subset = unordered_set<nodeIdx>()) // if inducing net
{
  bool loadEverything = subset.size() == 0;
  const string hwErrMsg =
  "ERROR: the system does not have a 4 byte uint and a 4 byte float.";

  const string fileErrMsg =
  "ERROR: the supplied graph files is invalid.";

  if(NUM_BYTE_PER_EDGE != 2 * sizeof(nodeIdx) + sizeof(float)){
    throw runtime_error(hwErrMsg);
  }

  //load graph path
  struct stat st;
  stat(graphPath.c_str(), &st);
  size_t totalFileSize = st.st_size;
  if(totalFileSize % NUM_BYTE_PER_EDGE){
    throw runtime_error(fileErrMsg);
  }
  size_t totalEdgeCount = totalFileSize / NUM_BYTE_PER_EDGE;

  #pragma omp parallel
  {

    size_t tid = omp_get_thread_num();
    size_t totalThreadNum = omp_get_num_threads();
    size_t edgesPerThread = totalEdgeCount / totalThreadNum;

    size_t startEdgeNum = tid * edgesPerThread;
    size_t endEdgeNum = startEdgeNum + edgesPerThread;
    if(tid == totalThreadNum-1){
      endEdgeNum = totalEdgeCount;
    }

    list<edge> localEdges;

    FILE* file;
    file = fopen(graphPath.c_str(), "rb");
    fseek(file, startEdgeNum * NUM_BYTE_PER_EDGE, SEEK_SET);
    charBuff buff[3];

    //load all the edges this thread is responsible for
    for(size_t i = startEdgeNum; i < endEdgeNum; ++i){
      fread(buff, sizeof(charBuff), 3, file);
      edge e(buff[0].i, buff[1].i, buff[2].f);

      if(loadEverything ||
         subset.find(e.a) != subset.end() ||
         subset.find(e.b) != subset.end())
      {
        localEdges.push_back(e);
      }
    }
    fclose(file);

    if(localEdges.size() > 0){
      //concatinate local vecs to the result
      #pragma omp critical
        result.splice(result.end(), localEdges);
    }
  }
}


size_t loadAnotherOrderNeighbors(const string& path,
                                       list<edge>& edges,
                                       unordered_set<nodeIdx>& allNodes){
  static size_t order = 0;
  order += 1;

  fastLoadEdgeList(path, edges, allNodes);

  for(const edge& e : edges){
    allNodes.insert(e.a);
    allNodes.insert(e.b);
  }
  return order;
}
