/*
 * Given the directory for the UMLS metathesurus data
 * generate a network of keywords
 *
 * Note: datafile ref: https://www.ncbi.nlm.nih.gov/books/NBK9685/
 */
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<utility>
#include<fstream>
#include<sstream>
#include<limits>
#include<algorithm>
#include<cmath>
#include<cstdio>
#include<stdlib.h>

#include"cmdln.h"
#include"parallelFileOp.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

class UmlsEdge{
public:
  UmlsEdge(const string& a, const string& b, float w_):w(w_){
    stringstream ss;
    if(a>b){
      ss << b << " " << a;
    } else {
      ss << a << " " << b;
    }
    key = ss.str();
  }
  string getKey() const{
    return key;
  }
  float getWeight() const{
    return w;
  }
private:
  string key;
  float w;
  friend ostream& operator<<(ostream&, const UmlsEdge&);
};

static const char delim = '|';

static const size_t IDA_COL_IDX = 0;
static const size_t IDB_COL_IDX = 4;
static const size_t REL_TYPE_COL_INX = 3;

static const float STRONG = 0.1f;
static const float MEDIUM = 0.5f;
static const float WEAK = 1.0f;

static const unordered_map<string, float> REL_2_WEIGHT{
  {"AQ", WEAK},    // allowed qualifier
  {"CHD", MEDIUM}, // child
  {"PAR", MEDIUM}, // parent
  {"QB", WEAK},    // can be qualified by
  {"RB", MEDIUM},  // broader relationship
  {"RL", STRONG},  // 'alike' or similar
  {"RN", MEDIUM},  // narrower
  {"RO", WEAK},    // OTHER
  {"RQ", STRONG},  // related or synonimous
  {"RU", WEAK},    // unspecified
  {"SIB", MEDIUM}, // sibling
  {"SY", STRONG}   // source asserted synonymy
};

vector<string> line2data(const string& line){
  stringstream ss(line);
  string tmp, a, b, rel;
  size_t count = 0, found = 0;
  while(getline(ss, tmp, delim)){
    switch(count){
      case IDA_COL_IDX: a = tmp; ++found; break;
      case IDB_COL_IDX: b = tmp; ++found; break;
      case REL_TYPE_COL_INX: rel = tmp; ++found; break;
      default: break;
    }
    ++count;
    if(found == 3) break;
  }
  return {a, b, rel};
}

bool isValidEdge(const string& line){
  const vector<string>& data = line2data(line);
  const string &a = data[0];
  const string &b = data[1];
  const string &rel = data[2];
  return (a != b && REL_2_WEIGHT.find(rel) != REL_2_WEIGHT.end());
}

UmlsEdge line2UmlsEdge(const string& line){
  const vector<string>& data = line2data(line);
  const string &a = data[0];
  const string &b = data[1];
  const string &rel = data[2];
  return UmlsEdge(a, b, REL_2_WEIGHT.at(rel));
}

int main(int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input",    'i', "path to MRREL.RRF", true);
  p.add<string>("output", 'o', "output edge file", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string inPath =  p.get<string>("input");
  string outPath =  p.get<string>("output");
  verbose = p.exist("verbose");

  vout << "Reading Edges" << endl;
  list<UmlsEdge> edgeList;
  fastProcFile<UmlsEdge>(inPath, edgeList, line2UmlsEdge, isValidEdge);

  vout << "Getting min unique edges" << endl;
  unordered_map<string, float> key2MinWeight;
  for(UmlsEdge& edge : edgeList){
    string key = edge.getKey();
    if(key2MinWeight.find(key) == key2MinWeight.end()){
      key2MinWeight.emplace(key, edge.getWeight());
    } else if(key2MinWeight.at(key) > edge.getWeight()){
      key2MinWeight[key] = edge.getWeight();
    }
  }

  vout << "writing" << endl;
  fstream outFile(outPath, ios::out);
  for(const auto& pair : key2MinWeight){
    outFile << pair.first << " " << pair.second << endl;
  }
  outFile.close();
}
