/*
 * Given the UMLS dataset, make edges to connect CUI to terms
 * I want to be able to run my merge code just like merging terms and centrods with tfidf
 * this set creates the 'tfidf' edges
 *
 * we are going to be working mostly with the mrconso data file
 *
 * we are calculating edit distances for each string to all dict strings... this will suck
 *
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
#include<cctype>
#include<cstdio>
#include<stdlib.h>
#include"cmdln.h"

using namespace std;


string cleanText(string text){
  text.erase(remove_if(text.begin(), text.end(), [](char c){return c != '-' && ispunct(c);}), text.end());
  //text.erase(remove_if(text.begin(), text.end(), [](char c){return isdigit(c);}), text.end());
  text.erase(unique(text.begin(), text.end(),
             [](char l, char r){return (l==r) && (l == ' ');}),
             text.end());
  text.erase(text.begin(),
             find_if(text.begin(),text.end(),
                     not1(ptr_fun<int,int>(isspace)))
            );
  text.erase(find_if(text.rbegin(), text.rend(), not1(ptr_fun<int,int>(isspace))).base(),
             text.end());
  transform(text.begin(), text.end(), text.begin(), [](char c){return tolower(c);});
  transform(text.begin(), text.end(), text.begin(), [](char c){return (c==' ' || c=='-' ? '_':c);});
  return text;
}

// Load from MRCONSO file
// column labels described here:
// https://www.ncbi.nlm.nih.gov/books/NBK9685/table/ch03.T.concept_names_and_sources_file_mr/?report=objectonly
struct KeywordData{
  const static int ID_COL_IDX = 0;
  const static int STR_COL_IDX = 14;
  string cui;
  string text;

  KeywordData(string data){
    vector<string> record = getRecord(data);
    cui = record[ID_COL_IDX];
    text = cleanText(record[STR_COL_IDX]);
  }

  vector<string> getRecord(string line, char delim = '|'){
    stringstream s(line);
    string tmp;
    vector<string> res;
    while(getline(s, tmp, delim)){
      res.push_back(tmp);
    }
    return res;
  }
};

struct edge{
  edge(string s1, string s2, float w):s1(s1),s2(s2),w(w){}
  string s1, s2;
  float w;
};

//  // map goes from raw text to CUI
//  pair<unordered_set<string>, int> getBestMatches(string str, unordered_map<string, string>& keyword2cui){
//    int minDist = numeric_limits<int>::max();
//    unordered_set<string> matches;
//    for(pair<string, string> p : keyword2cui){
//      int dist = edlibAlign(p.first.c_str(),
//                 p.first.size(),
//                 str.c_str(),
//                 str.size(),
//                 edlibDefaultAlignConfig()).editDistance;
//      // cout << str << " " << p.first << " " << dist << endl;
//      if(dist <= minDist){
//        if(dist < minDist){
//          minDist = dist;
//          matches.clear();
//        }
//        matches.insert(p.second);
//      }
//    }
//    return pair<unordered_set<string>, int>(matches, minDist);
//  }

int main(int argc, char** argv){

  cmdline::parser p;

  p.add<string>("umlsDir",    'd', "input directory of umls", true);
  p.add<string>("labelFile", 'l', "network label file", true);
  p.add<string>("outputFile", 'o', "output file edge list", true);
  p.add<float>("weight", 'w', "edge weight", false, 0);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string mrconsoPath =  p.get<string>("umlsDir") + "/META/MRCONSO.RRF";
  string labelPath =  p.get<string>("labelFile");
  string outputPath =  p.get<string>("outputFile");
  float weight =  p.get<float>("weight");
  bool verbose = p.exist("verbose");

  if(verbose) cout << "Started" << endl;

  if(verbose) cout << "READING: " << mrconsoPath << endl;

  unordered_map<string,string> keyword2cui;
  string line;
  fstream mrconsoFile(mrconsoPath, ios::in);

#pragma omp parallel
  {
#pragma omp single
  {
    while(getline(mrconsoFile, line)){
#pragma omp task firstprivate(line)
      {
        KeywordData data(line);
#pragma omp critical
        keyword2cui[data.text] = data.cui;
        if(verbose) cout << "READING: " << data.cui << " " << data.text << endl;
      }
    }
  }
  }

  if(verbose) cout << "closing mrconso" << endl;
  mrconsoFile.close();

  fstream termListFile(labelPath, ios::in);
  fstream outputFile(outputPath, ios::out);

  string term;

  if(verbose) cout << "Reading edges" << endl;
  vector<edge> edges;


#pragma omp parallel
  {
#pragma omp single
  {
    while(termListFile >> term){
#pragma omp task firstprivate(term)
      {
        string cleaned = cleanText(term);
        if(keyword2cui.find(cleaned) != keyword2cui.end()){
#pragma omp critical
          outputFile << term << " "<< keyword2cui[cleaned] << " " << weight << endl;
        }
      }
    }
  }
  }

  termListFile.close();
  outputFile.close();

  if(verbose) cout << "Done, cleaning up" << endl;
}
