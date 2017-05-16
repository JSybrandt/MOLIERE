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

  p.add<string>("netA", 'a', "input network A (int edge file)", true, "");
  p.add<string>("labelA", 'A', "input labels for network A", true, "");
  p.add<string>("netB", 'b', "input network B (int edge file)", true, "");
  p.add<string>("labelB", 'B', "input labels for network B", true, "");
  p.add<string>("netAB", 'c', "crossing edge file (string edge file)", true, "");
  p.add<string>("resultEdge", 'r', "output edge file", true, "");
  p.add<string>("resultLabel", 'R', "output label file", true, "");
  p.add<float>("netWeightA", 'x', "the weight for A edges", false, 1);
  p.add<float>("netWeightB", 'y', "the weight for B edges", false, 1);
  p.add<float>("netWeightAB", 'z', "the weight for AB edges", false, 1);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string netAPath =  p.get<string>("netA");
  string labelAPath =  p.get<string>("labelA");
  string netBPath =  p.get<string>("netB");
  string labelBPath =  p.get<string>("labelB");
  string netABPath =  p.get<string>("netAB");
  string resultEdgePath =  p.get<string>("resultEdge");
  string resultLabelPath =  p.get<string>("resultLabel");
  bool verbose = p.exist("verbose");
  float netWeightA = p.get<float>("netWeightA");
  float netWeightB = p.get<float>("netWeightB");
  float netWeightAB = p.get<float>("netWeightAB");

  if(verbose) cout << "Started opening files" << endl;
  fstream resEdgeFile(resultEdgePath, ios::out);
  fstream resLabelFile(resultLabelPath, ios::out);
  fstream netAFile(netAPath, ios::in);
  fstream labelAFile(labelAPath, ios::in);
  fstream netBFile(netBPath, ios::in);
  fstream labelBFile(labelBPath, ios::in);
  fstream netABFile(netABPath, ios::in);

  if(verbose) cout << "Reading label 1" << endl;

  unordered_map<string,unsigned long long> word2int;

  string label;
  unsigned long long lineNum = 0;
  while(labelAFile >> label){
    word2int[label] = lineNum;
    resLabelFile << label << endl;
    lineNum++;
  }

  if(verbose) cout << "Reading label 2" << endl;
  unsigned long long fileOffset = lineNum;
  while(labelBFile >> label){
    word2int[label] = lineNum;
    resLabelFile << label << endl;
    lineNum++;
  }

  if(verbose) cout << "Reading file 1" << endl;
  unsigned long long s,t;
  float w;
  while(netAFile >> s >> t >> w){
    resEdgeFile << s << " " << t << " " << w*netWeightA << endl;
  }

  if(verbose) cout << "Reading file 2" << endl;
  while(netBFile >> s >> t >> w){
    s += fileOffset;
    t += fileOffset;
    resEdgeFile << s << " " << t << " " << w*netWeightB << endl;
  }

  if(verbose) cout << "Reading AB file" << endl;
  string labelA, labelB;
  while(netABFile >> labelA >> labelB >> w){
    resEdgeFile << word2int[labelA] << " " << word2int[labelB] << " " << w*netWeightAB << endl;
  }

  if(verbose) cout << "Done, closing files" << endl;
  resEdgeFile.close();
  resLabelFile.close();
  netAFile.close();
  labelAFile.close();
  netBFile.close();
  labelBFile.close();
  netABFile.close();
}
