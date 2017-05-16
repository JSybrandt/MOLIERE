/*
 * TF-IDF
 *
 * Given the document files and term file, generate tf-idf edges
 *
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
#include<cstdio>
#include<stdlib.h>
#include"cmdln.h"

using namespace std;

typedef std::pair<unsigned int, unsigned int> edge;
struct edgeHash {
    inline std::size_t operator()(const edge & v) const {
        return v.first * 15485863 + v.second * 15485867;
   }
};
typedef std::unordered_set<edge, edgeHash> edgeSet;


int main(int argc, char** argv){

  cmdline::parser p;

  p.add<string>("docFile",    'd', "input doc file (1 doc per line, name is first word)", true, "");
  p.add<string>("termFile", 't', "input term file (1 term per line)", true, "");
  p.add<string>("outFile", 'o', "output edge file", true, "");
  p.add("verbose", 'v', "outputs debug information");
  p.add("normalize", 'n', "performs 0-1 scaling on edge weights");
  p.add("inverse", 'i', "calculates 1/tfidf for the purposes of shorestest path queries.");

  p.parse_check(argc, argv);

  string docPath =  p.get<string>("docFile");
  string termPath =  p.get<string>("termFile");
  string outPath =  p.get<string>("outFile");
  bool verbose = p.exist("verbose");
  bool normalize = p.exist("normalize");
  bool inverse = p.exist("inverse");

  if(verbose) cout << "Started" << endl;

  // Given: file with label & vec. Need to convert to FLANN format

  if(verbose) cout << "Loading Terms" << endl;
  fstream keyFile(termPath, ios::in);
  unordered_map<string,unsigned long long> term2DocCount;
  string term;
  while(keyFile >> term){
    term2DocCount[term] = 0;
  }
  keyFile.close();
  if(verbose) cout << "Loaded " << term2DocCount.size() << " terms." << endl;

  if(verbose) cout << "Counting terms in docs" << endl;
  fstream docFile(docPath, ios::in);

  unsigned int docCount = 0;
  string line;
  while(getline(docFile, line)){
    docCount++;
    stringstream s;
    s << line;
    string tmp, label;
    s >> label; //throw out label
    unordered_set<string> termSet;
    while(s >> tmp){
      // if word exists in the list
      if(term2DocCount.find(tmp) != term2DocCount.end()){
        termSet.insert(tmp);
        if(verbose) cout << "Noticed " << tmp << " in " << label << endl;
      }
    }
    for(string tmp : termSet) term2DocCount[tmp]++;
  }

  if(verbose) cout << "Finished Counting " << docCount << " documents" << endl;

  if(verbose) cout << "Going to start of docFile" << endl;
  docFile.clear();
  docFile.seekg(0, ios::beg);

  float dmin = numeric_limits<float>::infinity();
  float dmax = 0;

  fstream outFile(outPath, ios::out);

  while(getline(docFile, line)){
    stringstream s;
    s << line;
    string tmp, label;
    s >> label;
    unordered_map<string, unsigned int> term2Count;
    unsigned int totalTerms = 0;
    while(s >> tmp){
      // if term is valuable
      if(term2DocCount.find(tmp) != term2DocCount.end()){
        // if term not in current set
        if(term2Count.find(tmp) == term2Count.end()){
          term2Count[tmp] = 0;
        }
        term2Count[tmp]++;
        totalTerms++;
      }
    }
    for(pair<string, unsigned int> p : term2Count){
      // TF(t) = (Number of times term t appears in a document) / (Total number of terms in the document).
      // IDF(t) = log_e(Total number of documents / Number of documents with term t in it).
      // TFIDF = TF * IDF
      float result;
      if(inverse){
        result =  (float) totalTerms / (p.second * log(docCount / float(term2DocCount[p.first])));
      } else { // not inverse
        float tf = p.second / (float)totalTerms;
        float idf = log(docCount / float(term2DocCount[p.first]));
        result = tf * idf;
      }
      outFile << label << " " << p.first << " " << result << endl;
      // update min and max just incase we normalize later
      dmin = min(dmin, result);
      dmax = max(dmax, result);

    }
  }

  outFile.close();

  docFile.close();

  if(normalize){
    float dRange = dmax - dmin;
    if(verbose) cout << "Normalizing Data" << endl;
    int found = outPath.find_last_of("/");
    string oldPath = outPath.substr(0,found);
    string tmpPath = oldPath+"/.temp12345";
    if(verbose) cout << "Using " << tmpPath << endl << "Min: " << dmin << " Max:" << dmax << endl;

    fstream edgeFile(outPath, ios::in);
    fstream tmpFile(tmpPath, ios::out);

    string n1, n2;
    float val;
    while(edgeFile >> n1 >> n2 >> val){
      val = (val - dmin) / dRange;
      if(val < 0) val = 0; //prevents weird rounding issues
      tmpFile << n1 << " " << n2 << " " << val << endl;
    }
    tmpFile.close();
    edgeFile.close();
    if(verbose) cout << "Moving data to final location. ";
    system(("mv " + tmpPath + " " + outPath).c_str());
  }
  if(verbose) cout << "Done, cleaning up" << endl;
}
