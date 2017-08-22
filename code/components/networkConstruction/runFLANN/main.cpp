/*
 * Flann Runner
 *
 * Given a data file, this program will make a knn graph
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
#include"cmdln.h"

#include<flann/flann.hpp>

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

  p.add<string>("inFile",    'd', "input vector file", true, "");
  p.add<string>("graphFile", 'g', "output graph file", true, "");
  p.add<string>("labelFile", 'l', "output label file", true, "");
  p.add("verbose", 'v', "outputs debug information");
  p.add("normalize", 'n', "performs 0-1 scaling on edge weights");
  p.add("noSelfEdges", 'e', "Removes self edges with distance 0");
  p.add("ignoreFirstLine", 'i', "ignores first line of vector file. Useful for ft dicts.");
  p.add("noDuplicateEdges", 'u', "Removes only returns edges where i < j");
  p.add<unsigned int>("vecSize", 's', "dimensionality of vectors", false, 500);
  p.add<unsigned int>("numNN", 'k', "expected num nearest neighbors", false, 10);

  p.parse_check(argc, argv);

  string inPath =  p.get<string>("inFile");
  string graphPath =  p.get<string>("graphFile");
  string labelPath =  p.get<string>("labelFile");
  bool verbose = p.exist("verbose");
  bool normalize = p.exist("normalize");
  bool noSelfEdges = p.exist("noSelfEdges");
  bool noDuplicateEdges = p.exist("noDuplicateEdges");
  bool ignoreFirstLine = p.exist("ignoreFirstLine");
  unsigned int vecSize = p.get<unsigned int>("vecSize");
  unsigned int numNN = p.get<unsigned int>("numNN");

  if(verbose) cout << "Started" << endl;

  // Given: file with label & vec. Need to convert to FLANN format

  fstream dataFile(inPath, ios::in);

  vector<float>vecData;
  vector<string> labels;

  string line;
  while(getline(dataFile, line)){
    if(ignoreFirstLine){ignoreFirstLine = false; continue;}
    stringstream s;
    s << line;
    string label;
    float val;
    vector<float> vals;
    s >> label;
    labels.push_back(label);
    while(s >> val){
      vals.push_back(val);
    }
    if(vals.size() != vecSize){
      cerr << "Vector for label " << label << " is size " << vals.size() << endl
           << "Expected: " << vecSize << endl;
      return 1;
    }
    vecData.insert(vecData.end(), vals.begin(), vals.end());
  }
  dataFile.close();
  if(verbose) cout << "Loaded " << labels.size() << " labels." << endl;

  if(verbose) cout << "Running Flann" << endl;

  flann::Matrix<float> data(vecData.data(),labels.size(),vecSize);

  flann::Index<flann::L2<float> > index(data, flann::KDTreeIndexParams(16));
  index.buildIndex();

  vector<vector<int> > indicies;
  vector<vector<float> > dists;

  flann::SearchParams params(128);
  params.cores = 0; //automatic core selection
  index.knnSearch(data, indicies, dists, numNN,  params);

  if(verbose) cout << "Created Graph\n Nodes:" << indicies.size() << endl;

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

  if(verbose) cout << "Outputting Graph" << endl;

  fstream graphFile(graphPath, ios::out);
  edgeSet edges;
  for(unsigned int cNodeIdx = 0; cNodeIdx < indicies.size(); ++cNodeIdx){
      for(unsigned int j = 0; j < indicies[cNodeIdx].size(); ++j){
          unsigned int nNodeIdx = indicies[cNodeIdx][j];
          float dist = dists[cNodeIdx][j];

          bool isValid = true;
          if(isValid && noSelfEdges && cNodeIdx == nNodeIdx){
            isValid = false;
          }
          if(isValid && noDuplicateEdges){
            unsigned int s = min(cNodeIdx, nNodeIdx);
            unsigned int t = max(cNodeIdx, nNodeIdx);
            edge e(s,t);
            // if edge has been reported before
            if(edges.find(e) != edges.end()){
              isValid = false;
            } else{
              edges.insert(e);
            }
          }
          if(isValid)
            graphFile << cNodeIdx << " " << nNodeIdx << " " << dist << endl;
      }
  }
  graphFile.close();

  if(verbose) cout << "Outputting Labels" << endl;
  fstream labelFile(labelPath, ios::out);
  for(string label : labels)
    labelFile << label << endl;
  labelFile.close();
  if(verbose) cout << "Done, cleaning up" << endl;
}
