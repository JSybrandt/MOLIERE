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

bool defFilter(const string &){
  return true;
}

/*
 * This function will map each line from the file path and
 * map it using the provided function. The result is a list
 * of type T where each line corresponds to an entry in the list.
 * Note: Result is unordered.
 *
 * filePath - the path to parse in parallel. Each line is a record.
 * result - returned by refernce. New results are appended to this list.
 * mapper - each line of the input is run through this
 * filter - determines wether to process each line. Default accepts all.
 */
template<class T>
void fastProcFile(const string& filePath,
                  list<T>& result,
                  function<T(const string&)> mapper,
                  function<bool(const string&)> filter = defFilter)
{

  //get properties of abstract path
  struct stat st;
  stat(filePath.c_str(), &st);
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
    fstream localFile(filePath, ios::in | ios::binary);
    localFile.seekg(tid * bytesPerThread);

    string localLine;
    if(tid > 0){
      // jump to next newline
      getline(localFile, localLine);
    }

    fileStarts[tid] = localFile.tellg();

#pragma omp barrier

    list<T> locRes;

    // while we are still inside our own section
    unsigned int numLines = 0;
    while(localFile.tellg() < fileStarts[tid+1] && localFile){
      getline(localFile, localLine);
      numLines += 1;
      if(filter(localLine)){
        locRes.emplace_back(mapper(localLine));
      }
    }

    localFile.close();

#pragma omp critical
    {
      result.splice(result.end(), locRes);
    }

  }

}
