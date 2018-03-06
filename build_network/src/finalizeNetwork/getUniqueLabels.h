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

#include<omp.h>

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

/*
 * This function extracts the first two words of every line
 * and stores them in res. This is done in parallel.
 */

void getUniqueLabels(
    const string& edgeListPath,
    unordered_set<string>& res)
{

  //get properties of abstract path
  struct stat st;
  stat(edgeListPath.c_str(), &st);
  size_t totalFileSize = st.st_size;

  vector<size_t> fileStarts;

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
    fstream localFile(edgeListPath, ios::in | ios::binary);
    localFile.seekg(tid * bytesPerThread);

    string localLine;
    if(tid > 0){
      // jump to next newline
      getline(localFile, localLine);
    }

    fileStarts[tid] = localFile.tellg();

#pragma omp barrier

    /* UPDATED */
    unordered_set<string> localData;

    // while we are still inside our own section
    unsigned int numLines = 0;
    while(localFile.tellg() < fileStarts[tid+1] && localFile){
      getline(localFile, localLine);
      numLines += 1;

      /* UPDATED */
      stringstream ss(localLine);
      string tmp;
      ss >> tmp;
      if(localData.find(tmp) == localData.end())
        localData.emplace(move(tmp));
      ss >> tmp;
      if(localData.find(tmp) == localData.end())
        localData.emplace(move(tmp));

    }

    localFile.close();

#pragma omp critical
    {
      res.insert(make_move_iterator(localData.begin()),
                  make_move_iterator(localData.end()));
    }

  }

}

