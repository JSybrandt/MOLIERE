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
#include"parallelFileOp.h"
#include"stopword_list.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;


string procText(const string& line){
  stringstream reader(line);
  stringstream writer;
  string tmp;
  bool first = true;
  while(reader >> tmp){
    if(first){ // print pmid for debug
      vout << tmp << endl;
      first = false;
    }
    if(STOP_WORDS.find(tmp) == STOP_WORDS.end()){
      writer << tmp << " ";
    }
  }
  return writer.str();
}

bool isLong(const string& line){
  stringstream ss(line);
  string tmp;
  unsigned int count = 0;
  while(ss >> tmp){
    ++count;
    // PMID YEAR BREAK WORD BREAK == 5 words
    if(count > 5) return true;
  }
  return false;
}


int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("abstractFile", 'a', "abstract file (Run After Phrase Mining)", true);
  p.add<string>("outputFile", 'o', "output file", true);
  p.add("keep-short-abstracts", 'k', "keeps abstracts that only contain 1 word");
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string abstractPath =  p.get<string>("abstractFile");
  string outPath =  p.get<string>("outputFile");
  verbose = p.exist("verbose");
  bool keepShort = p.exist("keep-short-abstracts");

  vout << "Processing" << endl;

  list<string> processedText;
  if(keepShort){
    fastProcFile<string>(abstractPath, processedText, procText);
  } else {
    fastProcFile<string>(abstractPath, processedText, procText, isLong);
  }

  vout << "writing" << endl;
  fstream outFile(outPath, ios::out);
  for(const string& str : processedText){
    outFile << str << endl;
  }
  outFile.close();

  return 0;
}
