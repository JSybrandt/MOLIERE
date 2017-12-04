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

// needed for posix io
#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"cmdln.h"
#include"util.h"
#include"fixedSizeHeap.h"

#include<omp.h>

using namespace std;

bool verbose = false;

class invalidBinaryFile: public exception {};
class getLabelException: public exception {};
class invalidHardware: public exception {};

typedef unsigned int nodeIdx;

string getLabel(string path, nodeIdx idx){
  nodeIdx count = 0;
  fstream fin(path, ios::in);
  string token;
  while(fin >> token){
    if(count == idx){
      return token;
    }
    ++count;
  }
  throw getLabelException();
}

class getIdxException: public exception {};

nodeIdx getIdx(string path, string label){
  nodeIdx count = 0;
  fstream fin(path, ios::in);
  string token;
  while(fin >> token){
    if(token == label){
      return count;
    }
    ++count;
  }
  throw getIdxException();
}

void string2vec(const string& line, string& label, vector<float>& vec){
  stringstream ss(line);
  ss >> label;
  float temp;
  while(ss >> temp){ vec.push_back(temp);}
}

vector<float> getVector(string ngrams, string pmid, string umls, string label){
  string targetFile = ngrams;
  if(label[0] == 'C')
    targetFile = umls;
  if(label[0] == 'P')
    targetFile = pmid;
  fstream fin(targetFile, ios::in);
  string line, token;
  vector<float> res;
  while(getline(fin, line)){
    // line starts with label, not sufficient, but good filter
    if(strncmp(line.c_str(), label.c_str(), label.length()) == 0){
      stringstream ss(line);
      ss >> token;
      if(token == label){
        string2vec(line, label, res);
        return res;
      }
    }
  }
  cerr << "Failed to find " << label << " in " << targetFile << endl;
  throw getVectorException();
}

bool isInElipse(const vector<float>& fociiA,
                const vector<float>& fociiB,
                float elipseConst,
                const vector<float>& point){
  float d1 = distL2(point, fociiA);
  float d2 = distL2(point, fociiB);
  return d1 + d2 < elipseConst;
}

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

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("sourceLbl", 's', "id representing the source", true);
  p.add<string>("targetLbl", 't', "intended target", true);
  p.add<string>("outputFile", 'o', "Output neighborhood", true);
  p.add<string>("ngramVectors", 'V',  "File contanining text vectors for ngrams", true);
  p.add<string>("pmidCentroids", 'P', "File containing text vectors for PMIDs", true);
  p.add<string>("umlsCentroids", 'U', "File containing text vectors for UMLS terms", true);
  p.add<string>("abstractFile", 'a', "File of PMID [abstract content]", true);
  p.add<float>("elipseConst", 'e', "Constant alpha where distL2(A,B)*\\alpha = 2a", false, 1.4);
  p.add<unsigned int>("maxResult", 'm', "Maximum number of documents to return", false, 20000);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string sourceLbl =  p.get<string>("sourceLbl");
  string targetLbl =  p.get<string>("targetLbl");
  string outputPath =  p.get<string>("outputFile");
  string vectorPath = p.get<string>("ngramVectors");
  string pmidCentroidPath = p.get<string>("pmidCentroids");
  string umlsCentroidPath = p.get<string>("umlsCentroids");
  string abstractPath = p.get<string>("abstractFile");
  float elipseConstMultiple = p.get<float>("elipseConst");
  unsigned int maxResultSize = p.get<unsigned int>("maxResult");
  verbose = p.exist("verbose");

  vector<float> sourceVec, targetVec;
  float elipseConst;
  FixedSizeHeap<string, float> pmid2dist(maxResultSize);
  fstream outFile(outputPath, ios::out);

#pragma omp parallel
  {
#pragma omp single
  {

#pragma omp task
    sourceVec = getVector(vectorPath, pmidCentroidPath, umlsCentroidPath,  sourceLbl);

#pragma omp task
    targetVec = getVector(vectorPath, pmidCentroidPath, umlsCentroidPath, targetLbl);

#pragma omp taskwait
    elipseConst = distL2(sourceVec, targetVec) * elipseConstMultiple;

    if(verbose){
      cout << sourceLbl << "-> ";
      for(float f : sourceVec){
        cout << f << " ";
      }
      cout << endl;
      cout << targetLbl << "-> ";
      for(float f : targetVec){
        cout << f << " ";
      }
      cout << endl;
      cout << "Elipse Constant:" << elipseConst << endl;
    }

    fstream cFile(pmidCentroidPath, ios::in);
    string line;
    while(getline(cFile, line)){
#pragma omp task firstprivate(line)
      {
        vector<float> pt;
        string label;
        string2vec(line, label, pt);
        float dist = distL2(pt, sourceVec) + distL2(pt, targetVec);
        if (dist < elipseConst){
#pragma omp critical
          {
            if(verbose) cout << "In Elipse: " << label << endl;
            pmid2dist.insert(label, dist);
          }
        }
      }
    }
#pragma omp taskwait
    cFile.close();

    unordered_set<string> foundPMID;
    for(auto& pair: pmid2dist.getHeap()){
      foundPMID.insert(pair.first);
    }

    if(verbose) cout << "Writing BOW" << endl;

    fstream abstractFile(abstractPath, ios::in);
    while(getline(abstractFile, line)){
#pragma omp task firstprivate(line)
      {
        unsigned int space = line.find(' ');
        string pmid = line.substr(0, space);
        if(foundPMID.find(pmid) != foundPMID.end()){
          string bow =  abstractLine2Bow(line);
#pragma omp critical
          {
            if(verbose) cout << "Printing:" << pmid << endl;
            outFile << bow << endl;
          }
        }
      }
    }
#pragma omp taskwait
    abstractFile.close();
    outFile.close();
  }
  }


  return 0;
}
