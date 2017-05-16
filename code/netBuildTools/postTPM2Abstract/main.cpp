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

using namespace std;

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}
bool BothAreSpaces(char l, char r){return (l == r) && (l == ' ');}

int main(int argc, char** argv){

  cmdline::parser p;

  p.add<string>("postTPMFile", 't', "input directory of umls", true, "");
  p.add<string>("pmidFile", 'p', "input directory of umls", true, "");
  p.add<string>("outputFile", 'o', "output abstract file", true, "");
  p.add<string>("dummySentence" , 'd', "dummy phrase which we should throw out.", false, "quick_brown_fox");
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string postTPMPath =  p.get<string>("postTPMFile");
  string pmidPath =  p.get<string>("pmidFile");
  string outputPath =  p.get<string>("outputFile");
  string dummySentence =  p.get<string>("dummySentence");
  bool verbose = p.exist("verbose");

  if(verbose) cout << "Start" << endl;

  fstream postTPMFile(postTPMPath, ios::in);
  fstream pmidFile(pmidPath, ios::in);
  fstream outFile(outputPath, ios::out);

  string dataLine, pmid;
  while(getline(postTPMFile, dataLine) && getline(pmidFile, pmid)){
    pmid = trim(pmid);
    outFile << pmid;

    dataLine = trim(dataLine);
    // erase multiple spaces
    dataLine.erase(unique(dataLine.begin(), dataLine.end(), BothAreSpaces), dataLine.end());
    // spaces become _
    replace(dataLine.begin(), dataLine.end(), ' ','_');
    // , becomes space
    replace(dataLine.begin(), dataLine.end(), ',',' ');

    if(verbose) cout << dataLine << endl;

    // Idea: Some of the early PMIDs have been included b.c. numbers
    // so we are going to cut everything before the dummy sentence
    stringstream ss(dataLine);
    bool dumSentenceFound = false;
    string phrase;
    while(ss >> phrase){
      if(dumSentenceFound){
        outFile << " " << phrase;
      }
      else if(phrase == dummySentence){
        dumSentenceFound = true;
      }
    }
    outFile << endl;
  }

  postTPMFile.close();
  pmidFile.close();
  outFile.close();

  if(verbose) cout << "Done, cleaning up" << endl;
}
