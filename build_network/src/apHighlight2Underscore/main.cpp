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

const string START_TAG = "<phrase>";
const string END_TAG = "</phrase>";

string procText(const string& line){
  stringstream writer;
  bool inPhrase = false;
  for(unsigned int i = 0; i < line.size(); ++i){
    char c = line[i];
    if(inPhrase)
      if(isspace(c)) c = '_';
    if(c == '<'){
      if(line.substr(i, START_TAG.size()) == START_TAG){
        inPhrase = true;
        i += START_TAG.size() - 1;
        continue;
      }
      else if(line.substr(i, END_TAG.size()) == END_TAG){
        inPhrase = false;
        i += END_TAG.size() - 1;
        continue;
      }
    }
    writer << c;
  }

  //pass #2 stopwords:
  stringstream reader(writer.str());
  writer.str(string()); // clear

  string tmp;
  while(reader >> tmp){
    if(STOP_WORDS.find(tmp) == STOP_WORDS.end())
      writer << tmp << " ";
  }

  return writer.str();
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input", 'i', "AutoPhrase Highlight File", true);
  p.add<string>("output", 'o', "output file", true);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string inPath =  p.get<string>("input");
  string outPath =  p.get<string>("output");
  verbose = p.exist("verbose");

  vout << "Processing" << endl;

  list<string> res;
    fastProcFile<string>(inPath, res, procText);

  vout << "writing" << endl;
  fstream outFile(outPath, ios::out);
  for(const string& str : res){
    outFile << str << endl;
  }
  outFile.close();

  return 0;
}
