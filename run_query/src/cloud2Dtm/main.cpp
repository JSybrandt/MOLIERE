#include<iostream>
#include<unordered_map>
#include<map>
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
#include"util.h"
#include"bow.h"
#include"parallelFileOp.h"
#include"labelManager.h"
#include"files.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("output", 'o', "out root (-mult.dat -seq.dat -words.dat)", true);
  p.add<string>("input", 'i', "cloud file", true);
  p.add<string>("abstracts", 'a', "abstract file", true);
  p.add<string>("labels", 'l', "label file", true);
  p.add<size_t>("years-per-ts", 't', "number of years per time step", false, 1);
  p.add<size_t>("min-per-ts", 'm', "min docs per ts", false, 50);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string outRootPath =  p.get<string>("output");
  string inPath =  p.get<string>("input");
  string abPath =  p.get<string>("abstracts");
  string labelPath =  p.get<string>("labels");
  size_t yearsPerTs =  p.get<size_t>("years-per-ts");
  size_t minDocsPerTs =  p.get<size_t>("min-per-ts");
  verbose = p.exist("verbose");

  vout << "Checking input";
  assertIsFile(inPath);
  assertIsFile(abPath);
  assertIsFile(labelPath);
  vout << "Done";

  string wordPath = outRootPath + "-words.dat";
  string seqPath = outRootPath + "-seq.dat";
  string multPath = outRootPath + "-mult.dat";

  vout << "Loading labels...";
  LabelManager labels(labelPath);
  vout << "Done";

  vout << "Indexing cloud...";
  unordered_set<string> pmidSubset;
  fstream cloudFile(inPath, ios::in);
  nodeIdx tmp;
  while(cloudFile >> tmp)
    pmidSubset.insert(labels[tmp]);
  cloudFile.close();
  vout << "Done";

  auto selectAb = [&pmidSubset](const string& line) -> bool {
    stringstream ss(line);
    string pmid;
    ss >> pmid;
    return pmidSubset.find(pmid) != pmidSubset.end();
  };

  auto line2bagyear = [](const string& line) -> pair<Bow, size_t>{
    stringstream ss(line);
    string pmid;
    size_t year;
    ss >> pmid >> year;
    return {Bow(line, 2), year};
  };

  vout << "Loading cloud as bow...";
  list<pair<Bow, size_t>> bowYearList;
  fastProcFile<pair<Bow, size_t>>(abPath, bowYearList, line2bagyear, selectAb);
  vout << "Done";

  vout << "Indexing bags by ts...";
  map<size_t, list<Bow>> year2bags;
  for(auto& by : bowYearList)
    year2bags[by.second].emplace_back(move(by.first));

  size_t startYear = year2bags.begin()->first;
  map<size_t, list<Bow>> ts2bags;
  for(auto& ybs : year2bags){
    size_t ts = (ybs.first - startYear) / yearsPerTs;
    ts2bags[ts].splice(ts2bags[ts].begin(), ybs.second);
  }

  size_t validTsCount = 0;
  for(const auto& tsbl : ts2bags){
    if(tsbl.second.size() >= minDocsPerTs)
      validTsCount++;
  }

  vout << "Done" << endl;
  vout << "Identified " << validTsCount << " ts" << endl;

  vout << "Indexing words...";
  unordered_map<string, size_t> word2idx;
  fstream wordFile(wordPath, ios::out);
  for(const auto& tsbl : ts2bags){
    if(tsbl.second.size() < minDocsPerTs) continue;
    for(const Bow& b : tsbl.second){
      for(const auto& wc : b.getCounts()){
        // found new word
        if(word2idx.find(wc.first) == word2idx.end()){
          word2idx[wc.first] = word2idx.size();
          wordFile << wc.first << endl;
        }
      }
    }
  }
  wordFile.close();
  vout << "Done";

  vout << "Writing Seq File...";
  fstream seqFile(seqPath, ios::out);
  seqFile << validTsCount << endl;
  for(const auto& tsbl : ts2bags){
    if(tsbl.second.size() < minDocsPerTs) continue;
    seqFile << tsbl.second.size() << endl;
  }
  seqFile.close();
  vout << "Done";

  vout << "Writing Mult File...";
  fstream multFile(multPath, ios::out);
  for(const auto& tsbl : ts2bags){
    if(tsbl.second.size() < minDocsPerTs) continue;
    for(const Bow& b : tsbl.second){
      multFile << b.getCounts().size();
      for(const auto& wc : b.getCounts()){
        multFile << " " << word2idx[wc.first]
                 << ":" << wc.second;
      }
      multFile << endl;
    }
  }
  multFile.close();
  vout << "Done";

  return 0;
}
