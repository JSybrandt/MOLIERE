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
#include"pQueue.h"
#include"util.h"
#include"bow.h"
#include"parallelFileOp.h"
#include"labelManager.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

unordered_set<string> pmidSubset;
bool selectAbFromSubset(const string& line){
  stringstream ss(line);
  string label;
  ss >> label;
  return (pmidSubset.find(label) != pmidSubset.end());
}

pair<Bow, size_t> line2bowyear(const string& line){
  // bow skips pmid and year
  stringstream ss(line);
  string pmid, yearStr;
  ss >> pmid >> yearStr;
  return {Bow(ss.str(), 0, false), stoi(yearStr)};
}


int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("cloud", 'c', "file containing moliere IDs", true);
  p.add<string>("output", 'o', "Output file root (.mult .seq .words)", true);
  p.add<string>("labels", 'l', "Label file mapping moliere ID to string.", true);
  p.add<string>("abstracts", 'a', "abstract file", true);
  p.add<size_t>("years-per-timestep", 't', "Number of year to include in each ts", false, 1);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string cloudPath =  p.get<string>("cloud");
  string outPath =  p.get<string>("output");
  string labelPath =  p.get<string>("labels");
  string abstractPath =  p.get<string>("abstracts");
  size_t yearPerTs = p.get<size_t>("years-per-timestep");
  verbose = p.exist("verbose");

  string outSeqPath = outPath + "-seq.dat";
  string outMultPath = outPath + "-mult.dat";
  string outWordsPath = outPath + "-words.dat";

  vout << "Loading labels from " << labelPath << endl;
  LabelManager labels(labelPath);

  vout << "Loading cloud from " << cloudPath << endl;
  fstream cloudFile(cloudPath, ios::in);
  nodeIdx id;
  while(cloudFile >> id){
    pmidSubset.insert(labels[id]);
  }
  cloudFile.close();

  if(pmidSubset.size() == 0){
    throw runtime_error("Failed to load anything from cloud");
  }

  vout << "Loading abstracts in parallel from " << abstractPath << endl;
  vout << "Expecting to find... " << pmidSubset.size() << " pmids" << endl;
  list<pair<Bow, size_t>> bagYearList;
  fastProcFile<pair<Bow, size_t>>(abstractPath,
                                  bagYearList,
                                  line2bowyear,
                                  selectAbFromSubset);
  // want ordered map
  vout << "Mapping bags to years" << endl;
  map<size_t, list<Bow>> year2bows;
  for(auto& p : bagYearList){
    year2bows[p.second].emplace_back(move(p.first));
  }

  vout << "Mapping bags to timestamps" << endl;
  size_t minYear = year2bows.begin()->first;
  map<size_t, list<Bow>> timestamp2bows;
  for(auto& p : year2bows){
    size_t ts = (p.first - minYear) / yearPerTs;
    timestamp2bows[ts].splice(timestamp2bows[ts].end(), p.second);
  }

  vout << "Indexing and writing words" << endl;
  fstream outWordsFile(outWordsPath, ios::out);
  unordered_map<string, size_t> str2idx;
  for(const auto& ts2b : timestamp2bows){
    const list<Bow>& bags = ts2b.second;
    for(const auto& bag : bags)
      for(const auto& w2c : bag.getCounts()){
        if(str2idx.find(w2c.first) == str2idx.end()){
          outWordsFile << str2idx.size()
                       << "\t\"" << w2c.first << "\"" << endl;
          str2idx[w2c.first] = str2idx.size();
        }
      }
  }
  outWordsFile.close();

  vout << "Writing seq file" << endl;
  fstream outSeqFile(outSeqPath, ios::out);
  outSeqFile << timestamp2bows.size() << endl;
  for(const auto& t2bs : timestamp2bows){
    outSeqFile << t2bs.second.size() << endl;
  }
  outSeqFile.close();

  vout << "Writing mult file" << endl;
  fstream outMultFile(outMultPath, ios::out);
  for(const auto& t2bs : timestamp2bows){
    const auto& bags = t2bs.second;
    for(const Bow& bag : bags){
      // write # of words
      outMultFile << bag.getCounts().size();
      for(const auto& w2c : bag.getCounts()){
        outMultFile << " " << str2idx[w2c.first] << ":" << w2c.second;
      }
      outMultFile << endl;
    }
  }
  outMultFile.close();

  return 0;
}
