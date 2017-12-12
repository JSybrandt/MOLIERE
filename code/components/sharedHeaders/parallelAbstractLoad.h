#include<string>
#include<sstream>
#include<fstream>
#include<unordered_map>
#include<unordered_set>
#include<list>
#include<vector>

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

string abstractLine2Bow(string line){
  stringstream ss(line);
  string word;
  ss >> word; // throw away pmid
  unordered_map<string, unsigned int> bow;
  while(ss >> word){
    if(bow.find(word) == bow.end()){
      bow[word] = 1;
    } else {
      bow[word] += 1;
    }
  }
  stringstream out;
  for(auto& pair : bow){
    out << pair.first << " " << pair.second << " ";
  }
  return out.str();
}

string getLineFromBinary(fstream& binFile, size_t& byteCount){
  stringstream ss;
  char c;
  while(binFile >> c){
    ++byteCount;
    if(c == '\n') break;
    ss << c;
  }
  return ss.str();
}

void fastLoadAbtract2Bow(const string& abstractPath,
                         list<pair<string, string>>& result,
                         unordered_set<string> subset = unordered_set<string>()){

  bool takeAll = (subset.size() == 0);

  //get properties of abstract path
  struct stat st;
  stat(abstractPath.c_str(), &st);
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
    fstream localAbFile(abstractPath, ios::in | ios::binary);
    localAbFile.seekg(tid * bytesPerThread);

    string localLine;
    if(tid > 0){
      // jump to next newline
      getline(localAbFile, localLine);
    }

    fileStarts[tid] = localAbFile.tellg();

#pragma omp barrier

    list<pair<string, string>> localAb2Bow;

    // while we are still inside our own section
    unsigned int numLines = 0;
    while(localAbFile.tellg() < fileStarts[tid+1] && localAbFile){
      getline(localAbFile, localLine);
      numLines += 1;

      unsigned int space = localLine.find(' ');
      string pmid = localLine.substr(0, space);
      if(takeAll || subset.find(pmid) != subset.end()){
        string bow =  abstractLine2Bow(localLine);
        localAb2Bow.emplace_back(pmid, bow);
      }
    }

    localAbFile.close();

#pragma omp critical
    {
      result.splice(result.end(), localAb2Bow);
    }

  }

}

