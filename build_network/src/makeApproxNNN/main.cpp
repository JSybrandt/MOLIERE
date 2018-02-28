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

#include"cmdln.h"
#include"util.h"
#include"parallelFileOp.h"
#include<flann/flann.hpp>

using namespace std;

bool verbose;
#define vout if(verbose) cout

int main(int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input", 'i', "input vector file", true);
  p.add<string>("output", 'o', "output edge list", true);
  p.add("verbose", 'v', "outputs debug information");
  p.add("normalize", 'n', "performs 0-1 scaling on edge weights");
  p.add<unsigned int>("numNN", 'k', "expected num nearest neighbors", true);

  p.parse_check(argc, argv);

  string inPath =  p.get<string>("input");
  string outPath =  p.get<string>("output");
  ::verbose = p.exist("verbose");
  bool normalize = p.exist("normalize");
  unsigned int numNN = p.get<unsigned int>("numNN");

  vout << "Loading" << endl;
  list<pair<string, vector<float>>> lbl2vecList;
  fastProcFile<pair<string, vector<float>>>(inPath, lbl2vecList, line2vec);

  vout << "Reserving transfer space" << endl;
  size_t vecSize = lbl2vecList.begin()->second.size();
  vector<string> labels;
  labels.reserve(lbl2vecList.size());
  vector<float> vectors;
  vectors.reserve(lbl2vecList.size() * vecSize);

  vout << "Moving" << endl;
  for(auto& pair : lbl2vecList){
    if(pair.second.size() != vecSize)
      throw runtime_error("Illegal Vec Size:" + pair.first);
    labels.emplace_back(move(pair.first));
    for(float f : pair.second)
      vectors.emplace_back(f);
  }

  vout << "Running Flann" << endl;

  flann::Matrix<float> matrix(vectors.data(), labels.size(), vecSize);

  flann::Index<flann::L2<float>> index(matrix, flann::KDTreeIndexParams(16));
  index.buildIndex();

  vector<vector<int>> indicies;
  vector<vector<float>> dists;

  flann::SearchParams params(128);
  params.cores = 0; //automatic core selection
  index.knnSearch(matrix, indicies, dists, numNN,  params);

  vout << "Created Graph\n Nodes:" << indicies.size() << endl;

  if(normalize){
    if(verbose) cout << "Performing 0 - 1 scaling" << endl;
    float dmin = numeric_limits<float>::infinity();
    float dmax = 0;
    for(vector<float>& v : dists){
      for(float f : v){
        dmin = min(dmin, f);
        dmax = max(dmax, f);
      }
    }
    float drange = dmax - dmin;
    for(vector<float>& v : dists){
      for(float& f : v){
        f = (f - dmin) / drange;
      }
    }
  }

  vout << "Outputting Graph" << endl;

  fstream graphFile(outPath, ios::out);
  unordered_set<string> edges;
  for(nodeIdx cNode = 0; cNode < indicies.size(); ++cNode){
      for(unsigned int j = 0; j < indicies[cNode].size(); ++j){
          nodeIdx nNode = indicies[cNode][j];
          float nDist = dists[cNode][j];

          // no self edges
          if(cNode == nNode){
            continue;
          }

          unsigned int a = min(cNode, nNode);
          unsigned int b = max(cNode, nNode);
          stringstream key;
          key << a << " " << b;
          if(edges.find(key.str()) == edges.end()){
            edges.insert(key.str());
            graphFile << labels[a] << " "
                      << labels[b] << " "
                      << nDist << endl;
          }
      }
  }
  graphFile.close();
}
