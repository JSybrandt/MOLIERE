#pragma once
#include<string>
#include<sstream>
#include<fstream>
#include<unordered_map>
#include<unordered_set>
#include<list>
#include<vector>
#include<functional>

// needed for posix io
#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> // for truncate

#include<omp.h>

#include"util.h"

using std::string;
using std::stringstream;
using std::fstream;
using std::ios;
using std::unordered_map;
using std::unordered_set;
using std::list;
using std::pair;
using std::vector;
using std::function;
using std::runtime_error;

/*
 * This function extracts the first two words of every line
 * and stores them in res. This is done in parallel.
 */

void appendParallelEdges(
    string edgePath,
    const vector<edge>& edges)
{

  //get properties of edge file
  // this is the # of edges written so far
  struct stat st;
  stat(edgePath.c_str(), &st);
  size_t prevFileSize = st.st_size;
  size_t newFileSize = prevFileSize + edges.size() * NUM_BYTE_PER_EDGE;
  truncate(edgePath.c_str(), newFileSize);
  std::cout << "Setting new file size to " << newFileSize << std::endl;

#pragma omp parallel
  {
    unsigned int tid = omp_get_thread_num();
    unsigned int totalThreadNum = omp_get_num_threads();
    size_t edgesPerThread = edges.size() / totalThreadNum;

    size_t startEdge = edgesPerThread * tid;
    size_t endEdge = startEdge + edgesPerThread;
    if(tid == totalThreadNum - 1)
      endEdge = edges.size();

    size_t startByte = prevFileSize + startEdge * NUM_BYTE_PER_EDGE;
    size_t endByte = prevFileSize + endEdge * NUM_BYTE_PER_EDGE;
    //
    // each thread puts its start position
    fstream localFile(edgePath, ios::in | ios::out | ios::ate | ios::binary);
    localFile.seekp(startByte);

    #pragma omp critical
    std::cout << "Thread " << tid << " intended " << startByte
              << " actual " << localFile.tellp()
              << "-" << endByte << std::endl
              << "\tFirst Edge:" << edges[startEdge].a
              << " " << edges[startEdge].b << endl;


    for(size_t i = startEdge; i < endEdge; ++i){
      const edge& e = edges[i];
      charBuff a, b, w;
      a.i = e.a;
      b.i = e.b;
      w.f = e.weight;
      localFile.write(a.buff, 4);
      localFile.write(b.buff, 4);
      localFile.write(w.buff, 4);
      if(tid == 0 && startEdge == i){
        std::cout << localFile.tellp() << " " << e.a << " " << e.b << " " << e.weight << std::endl;
      }
    }
    localFile.close();

  }

}

