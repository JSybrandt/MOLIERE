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
#include"parallelAbstractLoad.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;


int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("cloudFile", 'c', "file containing moliere IDs", true);
  p.add<string>("outputFile", 'o', "Output bog of words", true);
  p.add<string>("labelFile", 'l', "Label file mapping moliere ID to string.", true);
  p.add<string>("abstractFile", 'a', "abstract file", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string cloudPath =  p.get<string>("cloudFile");
  string outputPath =  p.get<string>("outputFile");
  string labelPath =  p.get<string>("labelFile");
  string abstractPath =  p.get<string>("abstractFile");
  verbose = p.exist("verbose");

  vector<string> mid2Label;
  mid2Label.reserve(pow(2, 25));

  vout << "Loading labels from " << labelPath << endl;
  fstream labelFile(labelPath, ios::in);
  string label;
  while(labelFile >> label){
    mid2Label.push_back(label);
  }
  labelFile.close();


  vout << "Loading cloud from " << cloudPath << endl;
  unordered_set<string> pmids;
  fstream cloudFile(cloudPath, ios::in);
  nodeIdx id;
  while(cloudFile >> id){
    pmids.insert(mid2Label[id]);
  }
  cloudFile.close();


  vout << "Loading abstracts in parallel from " << abstractPath << endl;
  vout << "Expecting to find... " << pmids.size() << " pmids" << endl;
  list<pair<string, string>> pmid2bow;
  fastLoadAbtract2Bow(abstractPath, pmid2bow, pmids);

  vout << "Found " << pmid2bow.size() << " pmids" << endl;


  vout << "Writing to " << outputPath << endl;
  fstream outFile(outputPath, ios::out);
  for(const auto& pair : pmid2bow)
    outFile << pair.second << endl;
  outFile.close();

  return 0;
}
