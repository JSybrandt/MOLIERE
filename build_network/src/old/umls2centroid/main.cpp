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
#include"util.h"

using namespace std;


string cleanText(string text){
  for(char& c : text){
    c = tolower(c);
    if((c < 'a' || c > 'z') && (c < '0' || c > '9')){
      c = ' ';
    }
  }
  text.erase(unique(text.begin(), text.end(),
             [](char l, char r){return (l==r) && (l == ' ');}),
             text.end());
  transform(text.begin(), text.end(), text.begin(), [](char c){return (c==' ' ? '_' : c);});
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


int main(int argc, char** argv){

  cmdline::parser p;

  p.add<string>("umlsDir",    'd', "input directory of umls", true);
  p.add<string>("outputFile", 'o', "output centroids", true);
  p.add<string>("ngramVecs", 'n', "ngram vector file", true);
  p.add<unsigned int>("vecLength", 'l', "vec length from vector file", false, 500);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string mrconsoPath =  p.get<string>("umlsDir") + "/META/MRCONSO.RRF";
  string outputPath =  p.get<string>("outputFile");
  string ngramPath =  p.get<string>("ngramVecs");
  unsigned int vecLength = p.get<unsigned int>("vecLength");
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
        {
          keyword2cui[data.text] = data.cui;
          //if(verbose) cout << "READING: " << data.cui << " " << data.text << endl;
        }
      }
    }
  }
  }

  if(verbose) cout << "closing mrconso" << endl;
  mrconsoFile.close();

  unordered_map<string, vector<float>> cui2sum;
  unordered_map<string, float> cui2count;


  fstream fin(ngramPath, ios::in);
  getline(fin, line); // skip first
#pragma omp parallel
#pragma omp single
  while(getline(fin, line)){
#pragma omp task firstprivate(line)
    {
      stringstream ss(line);
      string token;
      ss >> token;
      token = cleanText(token);
      if(keyword2cui.find(token) != keyword2cui.end()){
        if(verbose){
#pragma omp critical(debug)
          cout << "Found match:" << token << endl;
        }
        vector<float> vec(vecLength);
        float temp;
        int count = 0;
        while(ss >> temp){ vec[count] = temp; ++count; }
#pragma omp critical (res)
        {
          string cui = keyword2cui.at(token);
          if(cui2sum.find(cui) == cui2sum.end()){
            cui2sum[cui] = vector<float>(vecLength, 0.0f);
            cui2count[cui] = 0;
          }
          cui2sum[cui] += vec;
          cui2count[cui] += 1;
        }
      }
    }
  }

  fstream out(outputPath, ios::out);
  for(auto & pair: cui2sum){
    const string& cui = pair.first;
    pair.second /= cui2count[cui];
    out << cui;
    for(float f : pair.second) out << " " << f;
    out << endl;
  }
  out.close();

  if(verbose){
    for(auto& pair : keyword2cui){
      const string& word = pair.first;
      const string& cui = pair.second;
      if(cui2count.find(cui) == cui2count.end()){
        cout << "FAILED: " << cui << endl;
        cout << "\t" << word << endl;
      }
    }
  }
}
