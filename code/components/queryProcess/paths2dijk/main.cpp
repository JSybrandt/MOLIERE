/*
 * THIS PROGRAM ASSUMES WE HAVE USED A DIFFERENT ALG TO FIND THE PATHS
 * AND WE TAKE THAT INFO TO GENERATE THE DIJK FILE
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

#include"cmdln.h"
#include"pQueue.h"

using namespace std;

bool verbose = false;

// TYPES
typedef unsigned int nodeIdx;
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
  float operator[](nodeIdx i) const{
    return edges.find(i)->second;
  }
};

struct path{
  path():start(UNDEFINED), end(UNDEFINED), weight(-1){}
  path(nodeIdx s, nodeIdx e):start(s), end(e), weight(0){}
  vector<nodeIdx> nodes;
  float weight;
  nodeIdx start,end;
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
  graphMap(string graphFilePath, nodeIdx abStartIdx, nodeIdx numAbstracts,
            unsigned int cloudSetN, unsigned int cloudSetC, unsigned int cloudSetK){
    this->abStartIdx = abStartIdx;
    this->numAbstracts = numAbstracts;
    this->cloudSetN = cloudSetN;
    this->cloudSetC = cloudSetC;
    this->cloudSetK = cloudSetK;
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
  }
  nodeIdx abStartIdx, numAbstracts;
  unsigned int cloudSetN, cloudSetC, cloudSetK;

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

  const graphNode& operator[](unsigned int i) const {
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

  void runDijkstra(nodeIdx start, const vector<nodeIdx>& ends){
    unordered_set<nodeIdx> goals;
    vector<bool> visited(this->size(),false);
    for(nodeIdx s : ends){
      goals.insert(s);
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
          forwardEdge fe(currIdx, edge.second + pathWeight);
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
    if(verbose) cout << "Adding " << id << endl;
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

path loadPath(string line, const graphMap& graph){
  path res;
  stringstream s(line);
  nodeIdx node;
  while(s >> node){
    res.nodes.push_back(node);
  }
  res.start = res.nodes[0];
  res.end = res.nodes[res.nodes.size()-1];
  res.weight = 0;
  for(unsigned int i = 0; i < res.nodes.size()-1; ++i){
    res.weight += graph[res.nodes[i]][res.nodes[i+1]];
  }
  return res;
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("graphFile", 'g', "input graph file", true);
  p.add<string>("pathFile", 'p', "input path file (idx)", true);
  p.add<string>("outputFile", 'o', "Output paths and neighborhoods", true);
  p.add("verbose", 'v', "outputs debug information");
  p.add<unsigned int>("neighSize", 'n', "number of nearby abstracts to include", false, 1000);
  p.add<string>("labelFile", 'l', "Label file accompanying the edges file. Used to count PMIDS", true);
  p.add<nodeIdx>("abstractOffset", 'b', "the index of the first abstract in the label file.", false, 0);
  p.add<unsigned int>("cloudSetN", 'N', "abstract cloud param: number of new abstracts adjacent to those on path.", false, 2000);
  p.add<unsigned int>("cloudSetC", 'C', "abstract cloud param: number of new abstracts from keyword overlap", false, 500);
  p.add<unsigned int>("cloudSetK", 'K', "abstract cloud param: number of new abstracts from keywords", false, 500);

  p.parse_check(argc, argv);

  string graphPath =  p.get<string>("graphFile");
  string pathPath =  p.get<string>("pathFile");
  string outputPath =  p.get<string>("outputFile");
  string labelPath =  p.get<string>("labelFile");
  verbose = p.exist("verbose");
  unsigned int neighSize = p.get<unsigned int>("neighSize");
  nodeIdx abstractOffset = p.get<nodeIdx>("abstractOffset");
  unsigned int cloudSetN = p.get<unsigned int>("cloudSetN");
  unsigned int cloudSetC = p.get<unsigned int>("cloudSetC");
  unsigned int cloudSetK = p.get<unsigned int>("cloudSetK");

  nodeIdx numAbstracts = countPMID(labelPath);

  // CONSTRUCTING GRAPH
  if(verbose) cout << "Constructing Graph" << endl;
  graphMap graph(graphPath, abstractOffset, numAbstracts, cloudSetN, cloudSetK, cloudSetC);
  if(verbose) cout << "Found " << graph.size() << " nodes" << endl;
  if(verbose) cout << "Node ids " << graph.abStartIdx << "-"
                   << graph.abStartIdx+graph.numAbstracts << " are abstracts" << endl;


  vector<path> paths;
  fstream pathFile(pathPath, ios::in);
  string line;
  while(getline(pathFile, line)){
    paths.push_back(loadPath(line, graph));
  }
  pathFile.close();

  // OUTPUT PATHS
  fstream outFile(outputPath, ios::out);

#pragma omp parallel for schedule(dynamic)
  for(unsigned int i = 0; i < paths.size(); ++i){
    path p = paths[i];
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
