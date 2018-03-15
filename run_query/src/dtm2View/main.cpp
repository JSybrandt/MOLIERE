#include<iostream>
#include<unordered_map>
#include<map>
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
#include"util.h"
#include"bow.h"
#include"parallelFileOp.h"
#include"labelManager.h"
#include"files.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

vector<string> getTopicMatPaths(const string& dirPath){
  vector<string> paths;

  auto toFileName = [](size_t i) -> string {
    stringstream ss;
    ss << "topic-";
    if(i < 100) ss << "0";
    if(i < 10) ss << "0";
    ss << i;
    ss << "-var-e-log-prob.dat";
    return ss.str();
  };

  size_t topicNum = 0;
  string path = join(dirPath,toFileName(topicNum));
  while(isFile(path)){
    paths.push_back(path);
    topicNum++;
    path = join(dirPath,toFileName(topicNum));
  }

  return paths;
}

size_t getNumTs(const string& seqPath){
  size_t res;
  fstream in(seqPath, ios::in);
  in >> res;
  in.close();
  return res;
}

typedef vector<vector<float>> matrix;

matrix loadTopicMat(const string& topicMatPath, size_t numTs, size_t numWords){
  vector<vector<float>> probs;
  probs.reserve(numWords);

  fstream in(topicMatPath, ios::in);
  float tmp;

  for(size_t w = 0; w < numWords; ++w){
    vector<float> row;
    row.reserve(numTs);
    for(size_t i = 0; i < numTs; ++i){
      in >> tmp;
      row.push_back(exp(tmp));
    }
    probs.emplace_back(move(row));
  }
  in.close();
  return probs;
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("output", 'o', "Output dir", true);
  p.add<string>("input", 'i', "DTM result dir", true);
  p.add<string>("dtm-input-root", 'd', "DTM training file root (-words.dat)");
  p.add<float>("topic-cutoff", 'c', "min prob for inclusion", false, 0.01);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string outDirPath =  p.get<string>("output");
  string inDirPath =  p.get<string>("input");
  string dtmInRootPath =  p.get<string>("dtm-input-root");
  float topicCutoff = p.get<float>("topic-cutoff");
  verbose = p.exist("verbose");

  assertIsDir(inDirPath);

  if(! isDir(outDirPath)){
    mkdir(outDirPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
  }

  string inSeqPath = dtmInRootPath + "-seq.dat";
  assertIsFile(inSeqPath);

  string inMultPath = dtmInRootPath + "-mult.dat";
  assertIsFile(inMultPath);

  string inWordsPath = dtmInRootPath + "-words.dat";
  assertIsFile(inWordsPath);

  string inLdaDirPath = join(inDirPath, "lda-seq");
  assertIsDir(inLdaDirPath);

  vout << "Getting # ts : ";
  size_t numTs = getNumTs(inSeqPath);
  vout << numTs << endl;

  vout << "Loading words : ";
  LabelManager labels(inWordsPath);
  vout << labels.size() << endl;

  vout << "Loading topic files : ";
  vector<string> topic2path = getTopicMatPaths(inLdaDirPath);
  vout << topic2path.size() << endl;

  vout << "Loading Mats : ";
  vector<matrix> topicMats(topic2path.size());

  #pragma omp parallel for
  for(size_t i = 0; i < topic2path.size(); ++i){
    matrix mat = loadTopicMat(topic2path[i], numTs, labels.size());
    topicMats[i] = (move(mat));
  }
  vout << "Done" << endl;

  vout << "Writing files : ";

  for(size_t ts = 0; ts < numTs; ++ts){
    stringstream ss;
    ss << ts << ".view";
    fstream out(join(outDirPath, ss.str()), ios::out);

    for(size_t topic = 0; topic < topic2path.size(); ++topic){
      float total = 0;
      map<float, vector<size_t>> score2words;

      for(size_t word = 0; word < labels.size(); ++word){
        const vector<float>& row = topicMats[topic][word];
        float p = row[ts];
        total += p;
        if(p > topicCutoff)
          score2words[p].push_back(word);
      }

      out << endl;
      out << "Topic:\t" << topic << " " << total << endl;
      out << endl;
      for(auto it = score2words.rbegin(); it!= score2words.rend(); it++){
        for(size_t w : it ->second){
          out << labels[w] << " " << it->first << endl;
        }
      }
    }
    out.close();
  }
  vout << "Done" << endl;

  return 0;
}
