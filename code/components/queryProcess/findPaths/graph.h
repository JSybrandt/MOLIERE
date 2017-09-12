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
#include<cstring>
#include<exception>

#include"pQueue.h"
#include"cmdln.h"

#include<omp.h>

using namespace std;

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

// we need to assume a sparse node index list
struct graphMap{

  const VecDict& dict;
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
