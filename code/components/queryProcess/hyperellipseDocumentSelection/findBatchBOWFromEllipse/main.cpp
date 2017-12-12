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
#include<cmath>
#include<cstring>
#include<exception>
#include<list>

// needed for posix io
#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"cmdln.h"
#include"util.h"
#include"fixedSizeHeap.h"
#include"files.h"
#include"parallelVectorLoad.h"

#include<omp.h>

using namespace std;

bool verbose = false;
#define vout if(::verbose) cout

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

void fastLoadAbtract2Bow(const string& abstractPath, unordered_map<string, string>& ab2bow){
#pragma omp parallel
  {
#pragma omp single
  {
    fstream abstractFile(abstractPath, ios::in);
    string line;
    while(getline(abstractFile, line)){
#pragma omp task firstprivate(line)
      {
        unsigned int space = line.find(' ');
        string pmid = line.substr(0, space);
        string bow =  abstractLine2Bow(line);
#pragma omp critical
        ab2bow[pmid] = bow;
      }
    }
#pragma omp taskwait
    abstractFile.close();
  }
  }
}

struct query{
  query(string lblA, string lblB, float ellipseMultiple, unsigned int maxResSize,
        const unordered_map<string, vector<float>>& lbl2vec)
       :lblA(lblA),
        lblB(lblB),
        pmid2dist(maxResSize),
        vecA(lbl2vec.at(lblA)),
        vecB(lbl2vec.at(lblB)){
    ellipseConst = distL2(vecA, vecB) * ellipseMultiple;
  };

  bool check(const vector<float>& vec, float& dist) const{
    dist = distL2(vec, vecA) + distL2(vec, vecB);
    return (dist < ellipseConst);
  }

  void add(const string& pmid, float dist){
    pmid2dist.insert(pmid, dist);
  }

  unordered_set<string> getPMID() const{
    unordered_set<string> res;
    for(auto& pair: pmid2dist.getHeap()){
      res.insert(pair.first);
    }
    return res;
  }

private:
    string lblA, lblB;
    float ellipseConst;
    FixedSizeHeap<string, float> pmid2dist;
    const vector<float>& vecA;
    const vector<float>& vecB;
};


int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("queryFile", 'q', "File containing pairs of labels.", true);
  p.add<string>("outDir", 'o', "Directory to place results", true);
  p.add<string>("ngramVectors", 'V',  "File contanining text vectors for ngrams", true);
  p.add<string>("pmidCentroids", 'P', "File containing text vectors for PMIDs", true);
  p.add<string>("umlsCentroids", 'U', "File containing text vectors for UMLS terms", true);
  p.add<string>("abstractFile", 'a', "File of PMID [abstract content]", true);
  p.add<float>("ellipseConst", 'e', "Constant alpha where distL2(A,B)*\\alpha = 2a", false, 1.4);
  p.add<unsigned int>("maxResult", 'm', "Maximum number of documents to return", false, 20000);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string queryPath = p.get<string>("queryFile");
  string outDirPath =  p.get<string>("outDir");
  string vectorPath = p.get<string>("ngramVectors");
  string pmidCentroidPath = p.get<string>("pmidCentroids");
  string umlsCentroidPath = p.get<string>("umlsCentroids");
  string abstractPath = p.get<string>("abstractFile");
  float ellipseConstMultiple = p.get<float>("ellipseConst");
  unsigned int maxResultSize = p.get<unsigned int>("maxResult");
  verbose = p.exist("verbose");

  if(!isDir(outDirPath)){
    throw std::runtime_error("Must supply valid directory for output.");
  }

  vout << "Loading pmid" << endl;
  list<pair<string, vector<float>>> tempList;
  fastLoadVecDir(pmidCentroidPath, tempList);

  vout << "PMID list to vector" << endl;
  vector<pair<string, vector<float>>> pmid2vec;
  pmid2vec.reserve(tempList.size());
  move(begin(tempList), end(tempList), back_inserter(pmid2vec));

  vout << "Loading cuid" << endl;
  unordered_map<string, vector<float>> cuid2vec;
  fastLoadVecFile(umlsCentroidPath, cuid2vec);

  vout << "Loading abstracts" << endl;
  unordered_map<string, string> ab2bow;
  fastLoadAbtract2Bow(abstractPath, ab2bow);

  vout << "Loading Complete!" << endl;

  fstream queryFile(queryPath, ios::in);
  string queryLine;
  while(getline(queryFile, queryLine)){
    stringstream ss(queryLine);
    string sourceLbl, verb, targetLbl, year;
    getline(ss, sourceLbl, '|');
    getline(ss, verb, '|');
    getline(ss, targetLbl, '|');
    getline(ss, year, '|');

    vout << "Looking for " << sourceLbl << " " << targetLbl << endl;

    if(cuid2vec.find(sourceLbl) == cuid2vec.end()){
      vout << "Failed to find:" << sourceLbl << endl;
      continue;
    }

    if(cuid2vec.find(targetLbl) == cuid2vec.end()){
      vout << "Failed to find:" << targetLbl << endl;
      continue;
    }

    try{
      query q(sourceLbl, targetLbl, ellipseConstMultiple, maxResultSize, cuid2vec);
#pragma omp parallel for schedule(dynamic)
      for(unsigned int i = 0; i < pmid2vec.size(); ++i){
        const string& pmid = pmid2vec[i].first;
        const vector<float>& pt = pmid2vec[i].second;
        float dist = 0;
        if(q.check(pt, dist)){
#pragma omp critical
          q.add(pmid, dist);
        }
      }
      vout << "Writing abstracts for " << sourceLbl << " " << targetLbl << endl;
      string outFileName = sourceLbl + "---" + targetLbl + "---" + year;
      string outPath = join(outDirPath, outFileName);
      fstream outFile(outPath, ios::out);
      for(const string& pmid : q.getPMID()){
        outFile << ab2bow[pmid] << endl;
      }
      outFile.close();
    } catch(...){
      vout << "Something bad happened..." << endl;
    }
  }
  queryFile.close();



  return 0;
}
