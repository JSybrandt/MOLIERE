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
#include<cmath>

#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"omp.h"

#include"cmdln.h"
#include"parallelFileOp.h"
#include"bow.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

bool skipSecond = false;


Bow line2bow(const string& line){
  return Bow(line, (skipSecond ? 2 : 1));
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input", 'i', "Document file. First word is label", true);
  p.add<string>("output", 'o', "tf-idf edges file", true);
  p.add("skip-second", 's', "skips second token, i.e. year in the ab file");
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string inPath =  p.get<string>("input");
  string outPath =  p.get<string>("output");
  verbose = p.exist("verbose");
  skipSecond = p.exist("skip-second");

  vout << "Processing" << endl;

  list<Bow> docList;
  fastProcFile<Bow>(inPath, docList, line2bow);
  vector<Bow> docVec{
    make_move_iterator(docList.begin()),
    make_move_iterator(docList.end())
  };

  Bow docOccurRates;
  fstream outFile(outPath, ios::out);

  #pragma omp parallel
  {
    Bow localDocOccurRates;
    #pragma omp for
    for(size_t i = 0; i < docVec.size(); ++i){
      localDocOccurRates.add(docVec[i].toOccurances());
    }
    #pragma omp critical
      docOccurRates.add(localDocOccurRates);
    #pragma omp barrier
    #pragma omp for
    for(size_t i = 0; i < docVec.size(); ++i){
      const auto& doc = docVec[i];
      for(const auto& pair : doc.getCounts()){
        // float termFreq = float(pair.second) / docVec[i].getTotalCount();
        float invDocFreq = log(docVec.size() / docOccurRates.getCount(pair.first));
        // although tf-idf is normal, we want to inverse this for the net
        // 1 / (tf * idf) = docSize / (wc * idf)
        float score = doc.getTotalCount() / (float(pair.second) * invDocFreq);
        #pragma omp critical
        outFile << doc.getName() << " " << pair.first << " " << score << endl;
      }
    }
  }

  outFile.close();

  return 0;
}
