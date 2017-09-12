/*
 * Query Runner
 * This program takes in a set of sources and a set of targets
 * The goal is to find the shortest path between them,
 * and a neighboorhood around the paths.
 *
 * Ideas:
 *  - It might make sense to output paths in a directory
 *    that would allow me to process the next part in parallel.
 *  - I also want to be able to take advantage of the fact that I just
 *    enforced integer node labels. That should greatly improve performance.
 *  - I also have lost the ability to distinguish between abstracts and keywords.
 *    So I need to make sure I come up with a way to represent this later.
 *  - I probably want to build neighboorhoods in parallel
 *
 */

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
#include<cmath>

#include"pQueue.h"
#include"cmdln.h"

using namespace std;

bool verbose = false;

// TYPES
typedef unsigned int nodeIdx;
typedef unordered_map<nodeIdx, vector<float>> VecDict;
const nodeIdx UNDEFINED = numeric_limits<nodeIdx>::max();

struct forwardEdge{
  forwardEdge(nodeIdx e, float w):prevIdx(e), pathWeight(w){}
  nodeIdx prevIdx;
  float pathWeight;
  bool operator<(const forwardEdge& other) const {
    return this->pathWeight < other.pathWeight;
  }
};

struct graphNode{
  graphNode(): prevPathIdx(UNDEFINED),
               distFromSource(-1){}
  unordered_map<nodeIdx,float> edges;
  nodeIdx prevPathIdx;
  float distFromSource;
};

struct path{
  path():start(UNDEFINED), end(UNDEFINED), weight(-1){}
  path(nodeIdx s, nodeIdx e):start(s), end(e), weight(0){}
  nodeIdx start,end;
  float weight;
  vector<nodeIdx> nodes;
};

fstream& operator<<(fstream& out, const path& p){
  out << "Start: " << p.start << " End: " << p.end << " Path: ";
  for(nodeIdx i : p.nodes){
    out << i << " ";
  }
  out << "Weight: "<< p.weight;
  return out;
}

struct graphMap{
  graphMap(string graphFilePath,
           VecDict& dict,
           unsigned int dictVecLength,
           nodeIdx abStartIdx,
           nodeIdx numAbstracts,
           unsigned int cloudSetN,
           unsigned int cloudSetC,
           unsigned int cloudSetK
          ):dict(dict),
            dictVecLength(dictVecLength),
            abStartIdx(abStartIdx),
            numAbstracts(numAbstracts),
            cloudSetN(cloudSetN),
            cloudSetC(cloudSetC),
            cloudSetK(cloudSetK)
  {
    fstream inFile(graphFilePath.c_str(),ios::in);
    nodeIdx start, end;
    float weight;
    while(inFile>>start>>end>>weight){
      addNode(start);
      addNode(end);
      nodes[start].edges[end] = weight;
      nodes[end].edges[start] = weight;
    }
    inFile.close();

    maxVecDist = sqrt(4 * dictVecLength);
  }
  VecDict& dict;
  unsigned int dictVecLength;
  nodeIdx abStartIdx, numAbstracts;
  unsigned int cloudSetN, cloudSetC, cloudSetK;
  float maxVecDist;

  bool isAbstract(nodeIdx id) const{
    nodeIdx diff = id - abStartIdx;
    return this->contains(id) && (diff >= 0 && diff < numAbstracts);
  }

  bool contains(nodeIdx id) const{
    return nodes.size() > id;
  }

  unsigned int size() const{
    return nodes.size();
  }

  graphNode& operator[](unsigned int i){
    return nodes[i];
  }

  // Must runDijk first
  path getPath(nodeIdx start, nodeIdx end) const{
    if(verbose) cout << "Getting Path from " << start << "-" << end << endl;
    path res(start,end);
    if(contains(start) && contains(end) && nodes[end].distFromSource > 0){
      if(verbose) cout << "\tFound Such a path" << endl;
      res.weight = nodes[end].distFromSource;
      stack<nodeIdx> stk;
      nodeIdx currNode = end;
      while(currNode != start && currNode != UNDEFINED){
        stk.push(currNode);
        currNode = nodes[currNode].prevPathIdx;
      }
      // If we found a path successfully
      if(currNode ==  start){
        res.nodes.push_back(start);
        while(!stk.empty()){
          res.nodes.push_back(stk.top());
          stk.pop();
        }
      }
    }
    return res;
  }

  const vector<float>* getVec(nodeIdx a){
    auto it = dict.find(a);
    if(it != dict.end())
      return &it->second;
    return nullptr;
  }
  vector<float> getApproxVec(nodeIdx a){
    const vector<float>* vec = getVec(a);
    if(vec!=nullptr){
      return *vec;
    }
    vector<float> res(dictVecLength, 0.0f);
    unsigned int count = 0;
    for(auto edge : nodes[a].edges){
      const vector<float>* vec = getVec(edge.first);
      if(vec){
        for(unsigned int i=0; i < dictVecLength; ++i){
          res[i] += vec->at(i);
        }
        ++count;
      }
    }
    if(count > 0){
      for(unsigned int i=0; i < dictVecLength; ++i){
        res[i] /= count;
      }
    }
    return res;
  }
  float getNormalizedDist(const vector<float>& vA, const vector<float>& vB){
    float res = 0;
    for(unsigned int i = 0; i < dictVecLength; ++i){
      res += pow(vA[i] - vB[i], 2);
    }
    return sqrt(res) / maxVecDist;
  }

  void runAStar(nodeIdx start, const vector<nodeIdx>& ends){
    unordered_set<nodeIdx> goals;
    vector<bool> visited(this->size(),false);
    for(nodeIdx s : ends){
      goals.insert(s);
    }

    vector<vector<float>> endVecs;
    for(nodeIdx goal : goals){
      endVecs.push_back(getApproxVec(goal));
    }

    pQueue<nodeIdx,forwardEdge> pQ;
    pQ.push(start,forwardEdge(start, 0));
    visited[start] = true;

    while(!pQ.empty() && goals.size() > 0){
      pair<nodeIdx, forwardEdge> cPair = pQ.pop();

      nodeIdx& prevIdx = cPair.second.prevIdx;
      nodeIdx& currIdx = cPair.first;
      float pathWeight = cPair.second.pathWeight;

      graphNode& cNode = nodes[currIdx];
      cNode.prevPathIdx = prevIdx;
      cNode.distFromSource = pathWeight;

      //update goals
      goals.erase(currIdx);
      visited[currIdx] = true;

      for(pair<nodeIdx,float> edge : cNode.edges){
        if(!visited[edge.first]){
          float minDist2Goal = 999999;
          for(vector<float>& vec : endVecs){
            float dist = getNormalizedDist(getApproxVec(edge.first), vec);
            if(dist < minDist2Goal) minDist2Goal = dist;
          }
          forwardEdge fe(currIdx, edge.second + pathWeight + minDist2Goal);
          pQ.push(edge.first, fe); //relies on push updating min
        }
      }
    }
  }

  unordered_set<nodeIdx> getAbstractCloud(const path& p, unsigned int size) const{
    if(verbose){
      cout << "Fetching cloud for path:";
      for(nodeIdx n : p.nodes)
        cout << n << "-";
      cout << endl;
    }
    unordered_set<nodeIdx> res;
    for(unsigned int i = 0; i < p.nodes.size(); ++i){
      nodeIdx currNode = p.nodes[i];

      if(isAbstract(currNode)){ // get cloud set N
        if(verbose) cout << currNode << " is abstract" << endl;
        unordered_set<nodeIdx> set;
        pQueue<nodeIdx,float> pQ;
        pQ.push(currNode, 0);
        while(!pQ.empty() && set.size() < this->cloudSetN){
          pair<nodeIdx, float> cPair = pQ.pop();
          nodeIdx cIdx = cPair.first;
          float cWeight = cPair.second;
          set.insert(cIdx);
          for(pair<nodeIdx, float> e : nodes[cIdx].edges){
            // If we have yet to add the node to the neighborhood
            if(isAbstract(e.first) && set.find(e.first) == set.end())
              pQ.push(e.first, e.second + cWeight);
          }
        }
        res.insert(set.begin(), set.end());
      } else { // get cloud set K
        if(verbose) cout << currNode << " is keyword" << endl;
        pQueue<nodeIdx, float> pQ;
        for(pair<nodeIdx, float> e : nodes[currNode].edges){
          if(isAbstract(e.first)) pQ.push(e.first, e.second);
        }
        unsigned int addCount = 0;
        while(!pQ.empty() && addCount < this->cloudSetK){
          res.insert(pQ.pop().first);
          addCount++;
        }
      }
      //if there are two adjacent keywords
      // get cloud set C
      if(i+1 < p.nodes.size() &&
          !isAbstract(currNode) &&
          !isAbstract(p.nodes[i+1])){
        nodeIdx nextNode = p.nodes[i+1];
        if(verbose) cout << currNode << " also has an adj keyword " << nextNode << endl;
        pQueue<nodeIdx, float> pQ;
        for(pair<nodeIdx, float> e : nodes[currNode].edges){
          if(verbose) cout << "checking during intersection: " << e.first << endl;
          if(isAbstract(e.first) &&
              (nodes[nextNode].edges.find(e.first) != nodes[nextNode].edges.end())){
            if(verbose) cout<< "\t interfound " << e.first << endl;
            pQ.push(e.first, (e.second + nodes[nextNode].edges.at(e.first)) / 2);
          }
        }
        unsigned int addCount = 0;
        while(!pQ.empty() && addCount < this->cloudSetC){
          res.insert(pQ.pop().first);
          addCount++;
        }
      }
    }
    return res;
  }

private:
  void addNode(nodeIdx id){
    if(verbose && nodes.size() < id) cout << "Adding " << id << endl;
    while(nodes.size() <= id)
      nodes.push_back(graphNode());
  }
  vector<graphNode> nodes;
};

vector<nodeIdx> parseTargets(string targetPath){
  fstream tFile(targetPath, ios::in);
  nodeIdx target;
  vector<nodeIdx> res;
  while(tFile >> target)
    res.push_back(target);
  tFile.close();
  return res;
}

nodeIdx countPMID(string labelFilePath){
  fstream lFile(labelFilePath, ios::in);
  string label;
  nodeIdx count = 0;
  while(lFile >> label){
    if(label.substr(0,4) == "PMID"){
      count++;
    } else {
      break;
    }
  }
  lFile.close();
  return count;
}

unsigned int loadVectorDict(string path, VecDict& dict, string lblPath){
  string token, line;
  unsigned int numVectors, vecLength, count;
  float val;
  unordered_map<string, nodeIdx> labels;
  fstream lblFile(lblPath, ios::in);
  count = 0;
  while(lblFile >> token){
    if(token.size()>0){
      if(token[0] != 'P' && token[0] != 'C'){
        labels[token] = count;
      }
      ++count;
    }
  }
  lblFile.close();

  fstream dFile(path, ios::in);

  dFile >> numVectors >> vecLength;
  getline(dFile, line); // remember, we need to eat that whitespace
  while(getline(dFile, line)){
    stringstream ss(line);
    ss >> token;
    if(labels.find(token) == labels.end())
      continue;
    vector<float>& vec = dict[labels[token]];
    vec = vector<float>(vecLength, 0.0f);
    count = 0;
    while(ss >> val){
      vec[count] = val;
      ++count;
    }
  }
  dFile.close();
  return vecLength;
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("graphFile", 'g', "input graph file", true, "");
  p.add<nodeIdx>("sourceIdx", 's', "id representing the source", true);
  p.add<nodeIdx>("targetIdx", 't', "intended target", false, UNDEFINED);
  p.add<string>("intendedTargets", 'T', "intended targets", false, "");
  p.add<string>("outputFile", 'o', "Output paths and neighborhoods", true, "");
  p.add<string>("dictFile", 'd', "Dict File for vecors", true, "");
  p.add("verbose", 'v', "outputs debug information");
  p.add<unsigned int>("neighSize", 'n', "number of nearby abstracts to include", false, 1000);
  p.add<nodeIdx>("numAbstracts", 'a', "number of abstracts in the network", false, 0);
  p.add<string>("labelFile", 'l', "Label file accompanying the edges file. Used to count PMIDS", false, "");
  p.add<nodeIdx>("abstractOffset", 'b', "the index of the first abstract in the label file.", false, 0);
  p.add<unsigned int>("cloudSetN", 'N', "abstract cloud param: number of new abstracts adjacent to those on path.", false, 2000);
  p.add<unsigned int>("cloudSetC", 'C', "abstract cloud param: number of new abstracts from keyword overlap", false, 500);
  p.add<unsigned int>("cloudSetK", 'K', "abstract cloud param: number of new abstracts from keywords", false, 500);

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graphFile");
  nodeIdx sourceIdx =  p.get<nodeIdx>("sourceIdx");
  string targetPath =  p.get<string>("intendedTargets");
  nodeIdx targetIdx =  p.get<nodeIdx>("targetIdx");
  string outputPath =  p.get<string>("outputFile");
  string labelPath =  p.get<string>("labelFile");
  string dictPath = p.get<string>("dictFile");
  verbose = p.exist("verbose");
  unsigned int neighSize = p.get<unsigned int>("neighSize");
  nodeIdx numAbstracts = p.get<nodeIdx>("numAbstracts");
  nodeIdx abstractOffset = p.get<nodeIdx>("abstractOffset");
  unsigned int cloudSetN = p.get<unsigned int>("cloudSetN");
  unsigned int cloudSetC = p.get<unsigned int>("cloudSetC");
  unsigned int cloudSetK = p.get<unsigned int>("cloudSetK");

  if(targetIdx != UNDEFINED && targetPath != ""){
    cerr << "Cannot specify both a single and multiple targets. Pick one" << endl;
    exit(1);
  }
  if(labelPath == "" && numAbstracts == 0){
    cerr << "Must supply either the path to the graph's label file, or the number of abstracts." << endl;
    exit(1);
  }

  if(dictPath != "" && labelPath == ""){
    cerr << "Cannot specificy a term dictionary without specifying lablels." << endl;
    exit(1);
  }

  if(verbose) cout << "Loading vector dict from " << dictPath << endl;
  VecDict dict;
  unsigned int vecLength = loadVectorDict(dictPath, dict, labelPath);
  if(verbose) cout << "Loaded " << dict.size() << " vectors" << endl;

  if(labelPath != ""){
    nodeIdx count = countPMID(labelPath);
    if(numAbstracts != 0 && numAbstracts != count){
      cerr << "Warning, user supplied #abstracts = " << numAbstracts
           << " but supplied a label file with " << count << " abstracts.";
    }
    numAbstracts = count;
  }

  // CONSTRUCTING GRAPH
  if(verbose) cout << "Constructing Graph" << endl;
  graphMap graph(graphPath, dict, vecLength, abstractOffset, numAbstracts, cloudSetN, cloudSetK, cloudSetC);
  if(verbose) cout << "Found " << graph.size() << " nodes" << endl;
  if(verbose) cout << "Node ids " << graph.abStartIdx << "-"
                   << graph.abStartIdx+graph.numAbstracts << " are abstracts" << endl;

  // GETTING TARGETS
  vector<nodeIdx> targets;
  if(targetIdx != UNDEFINED){
    targets.push_back(targetIdx);
  } else {
    if(targetPath != ""){
      if(verbose) cout << "Getting Targets from " << targetPath << endl;
      targets = parseTargets(targetPath);
    } else {
      if(verbose) cout << "Looking for all targets. No file specified." << endl;
      for(nodeIdx i = 0; i < graph.size(); ++i){
        targets.push_back(i);
      }
    }
  }
  if(verbose) cout << "Identified " << targets.size() << " targets." << endl;

  // TRAVERSAL DIJKSTRA
  if(verbose) cout << "Starting Traversal" << endl;
  graph.runAStar(sourceIdx, targets);

  // OUTPUT PATHS
  fstream outFile(outputPath, ios::out);

#pragma omp parallel for schedule(dynamic)
  for(unsigned int i = 0; i < targets.size(); ++i){
    path p = graph.getPath(sourceIdx, targets[i]);
    if(p.nodes.size() > 1){
      if(verbose) cout << "outputing " << p.start << "-" << p.end << endl;
      unordered_set<nodeIdx> neighborhood = graph.getAbstractCloud(p, neighSize);
#pragma omp critical
{
      outFile << p << endl;
      for(nodeIdx neigh : neighborhood){
        outFile << neigh << " ";
      }
      outFile << endl;
}
    }
  }
  outFile.close();
  return 0;
}
