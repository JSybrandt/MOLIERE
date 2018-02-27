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
#include<regex>

#include<cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include"cmdln.h"
#include"parallelFileOp.h"
#include"cleanText.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

unsigned int phraseCol;

pair<string, string> line2idText(const string& line){
  unsigned int col = 1;
  pair<string, string> res;
  stringstream reader(line);
  getline(reader, res.first, '|'); ++col;
  while(col < phraseCol){
    getline(reader, res.second, '|'); ++col;
  }
  res.second = cleanText(res.second);
  // remove trailing clauses
  regex re("\\.\\s.*$");
  regex_replace(res.second, re, "");
  return res;
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input-file", 'i', "MRCONSO file", true);
  p.add<string>("output-file", 'o', "output file", true);
  p.add<unsigned int>("phrase-column", 'c', "1 indexed column", false, 16);
  p.add("verbose", 'v', "outputs debug information");
  p.parse_check(argc, argv);

  string inputPath =  p.get<string>("input-file");
  string outPath =  p.get<string>("output-file");
  phraseCol = p.get<unsigned int>("phrase-column");
  verbose = p.exist("verbose");

  if(phraseCol <= 1){
    throw runtime_error("phrase-column must be greater than 1");
  }

  vout << "Processing" << endl;

  list<pair<string, string>> id2textList;
  fastProcFile<pair<string, string>>(inputPath, id2textList, line2idText);

  unordered_map<string, unordered_set<string>> id2texts;
  for(const auto& pair : id2textList){
    id2texts[pair.first].insert(pair.second);
  }

  vout << "writing" << endl;
  fstream outFile(outPath, ios::out);
  for(const auto& pair : id2texts){
    outFile << pair.first;
    for(const string& s : pair.second)
      outFile << " " << s;
    outFile<< endl;
  }
  outFile.close();

  return 0;
}
