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
#include"pQueue.h"
#include"util.h"
#include"bow.h"
#include"parallelFileOp.h"
#include"labelManager.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("abstracts", 'a', "abstract file", true);
  p.add<string>("input", 'i', "word file", true);
  p.add<string>("output", 'o', "word + count file", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string abstractPath =  p.get<string>("abstracts");
  string inPath =  p.get<string>("input");
  string outPath =  p.get<string>("output");
  verbose = p.exist("verbose");

  unordered_set<string> words;
  string tmp;
  fstream inFile(inPath, ios::in);
  while(inFile >> tmp){
    words.emplace(move(tmp));
  }
  inFile.close();

  const auto& line2counts = [&words](const string& line) -> Bow {
    Bow bag;
    stringstream ss(line);
    string tmp;
    while(ss >> tmp)
      if(words.find(tmp) != words.end())
        bag.add(tmp);
    return bag;
  };

  list<Bow> bagList;
  fastProcFile<Bow>(abstractPath, bagList, line2counts);
  vector<Bow> bagVec;
  bagVec.reserve(bagList.size());
  for(auto& b : bagList)
    bagVec.emplace_back(move(b));

  Bow totalBag;
  #pragma omp parallel
  {
    Bow localBag;
    #pragma omp for
    for(size_t i = 0; i < bagVec.size(); ++i){
      localBag.add(bagVec[i]);
    }
    #pragma omp critical
    totalBag.add(localBag);
  }

  fstream outFile(outPath, ios::out);
  for(auto p : totalBag.getCounts()){
    outFile << p.first << " " << p.second << endl;
  }
  outFile.close();

  return 0;
}
