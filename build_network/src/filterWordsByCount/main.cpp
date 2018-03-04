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
#include<exception>
#include<list>

#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"cmdln.h"
#include"parallelFileOp.h"
#include"bow.h"

bool verbose = false;
#define vout if(::verbose) cout

bool skipSecond = false;
bool removePeriod = false;

unordered_set<string> goodWords;

using namespace std;

Bow line2bow(const string& line){
  return Bow(line, (skipSecond ? 2 : 1));
}

string line2filteredLine(const string& line){
  stringstream reader(line);
  stringstream writer;
  string tmp;
  reader >> tmp; // always skip first
  writer << tmp << " ";
  if(skipSecond){ // sometimes skip second
    reader >> tmp;
    writer << tmp << " ";
  }
  while(reader >> tmp){
    if(tmp == "." && removePeriod) continue;
    if(goodWords.find(tmp) != goodWords.end())
      writer << tmp << " ";
  }
  return writer.str();
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input", 'i', "each line of this file is a record", true);
  p.add<string>("output", 'o', "output file", true);
  p.add<size_t>("min-count", 'm', "minimum support to be included", true);
  p.add("verbose", 'v', "outputs debug information");
  p.add("skip-second", 's', "skips the second word of each record (in addition to the first)");
  p.add("remove-period", 'r', "removes sentence separator '.' from record");

  p.parse_check(argc, argv);

  string inputPath =  p.get<string>("input");
  string outPath =  p.get<string>("output");
  size_t minCount =  p.get<size_t>("min-count");
  verbose = p.exist("verbose");
  skipSecond = p.exist("skip-second");
  removePeriod = p.exist("remove-period");

  vout << "Processing" << endl;

  list<Bow> bagList;
  fastProcFile<Bow>(inputPath, bagList, line2bow);

  vector<Bow> bags{
    make_move_iterator(bagList.begin()),
    make_move_iterator(bagList.end())
  };

  vout << "Tabulating in parallel" << endl;
  Bow totalCounts;
#pragma omp parallel
  {
    Bow localCounts;
    #pragma omp for
    for(size_t i = 0; i < bags.size(); ++i){
      localCounts.add(bags[i]);
    }

    unordered_set<string> localGoodWords;
    Bow localBadWords;
    for(const auto& pair : localCounts.getCounts()){
      if(pair.second >= minCount)
        localGoodWords.insert(pair.first);
      else // later, this may become a good word
        localBadWords.add(pair.first, pair.second);
    }
    #pragma omp critical(counts)
    totalCounts.add(localBadWords);
    #pragma omp critical(goodWords)
    goodWords.insert(localGoodWords.begin(),localGoodWords.end());
  }

  vout << "Only keeping words with sufficient count" << endl;
  for(const auto& pair : totalCounts.getCounts()){
    if(pair.second >= minCount)
      goodWords.emplace(pair.first);
  }

  list<string> filteredText;
  fastProcFile<string>(inputPath, filteredText, line2filteredLine);

  vout << "writing" << endl;
  fstream outFile(outPath, ios::out);
  for(const string& str : filteredText){
    outFile << str << endl;
  }
  outFile.close();

  return 0;
}
