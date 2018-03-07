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

unordered_set<string> pmidSubset;
bool selectAbFromSubset(const string& line){
  stringstream ss(line);
  string label;
  ss >> label;
  return (pmidSubset.find(label) != pmidSubset.end());
}

Bow line2bow(const string& line){
  // bow skips pmid and year
  return Bow(line, 2);
}


int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("cloud", 'c', "file containing moliere IDs", true);
  p.add<string>("output", 'o', "Output bog of words", true);
  p.add<string>("labels", 'l', "Label file mapping moliere ID to string.", true);
  p.add<string>("abstracts", 'a', "abstract file", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string cloudPath =  p.get<string>("cloud");
  string outputPath =  p.get<string>("output");
  string labelPath =  p.get<string>("labels");
  string abstractPath =  p.get<string>("abstracts");
  verbose = p.exist("verbose");

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
  list<Bow> bags;
  fastProcFile<Bow>(abstractPath, bags, line2bow, selectAbFromSubset);

  vout << "Found " << bags.size() << " pmids" << endl;

  vout << "Writing to " << outputPath << endl;
  fstream outFile(outputPath, ios::out);
  for(Bow& bag : bags){
    bag.removeName();
    outFile << bag << endl;
  }
  outFile.close();

  return 0;
}
