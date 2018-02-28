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
#include"cleanText.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;


int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input", 'i', "each line of this file is a record", true);
  p.add<string>("output", 'o', "output file", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string inputPath =  p.get<string>("input");
  string outPath =  p.get<string>("output");
  verbose = p.exist("verbose");

  vout << "Processing" << endl;

  list<string> processedText;
  fastProcFile<string>(inputPath, processedText, cleanText);

  vout << "writing" << endl;
  fstream outFile(outPath, ios::out);
  for(const string& str : processedText){
    outFile << str << endl;
  }
  outFile.close();

  return 0;
}
