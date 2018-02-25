#pragma once

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
#include<stack>
#include<list>

#include"pQueue.h"
#include"util.h"

#include<omp.h>

using namespace std;

typedef unsigned int nodeIdx;

class graph{
public:
  graph(string labelPath){
    fstream lFile(labelPath, ios::in);
    string token;
    nodeIdx count = 0;
    while(lFile >> token){
      if(token[0] == 'P')
        abstractIdx.insert(count);
      else if(token[0] == 'C')
        umlsIdx.insert(count);
      ++count;
    }
  }
  unsigned int numNodes() const { return data.size();}
  unsigned int numEdges() const { return edgeCount;}
  void addEdge(nodeIdx a, nodeIdx b, float weight){
    try{
      if(weight < 0)
        throw runtime_error("Found negative edge weight");
      if(a != UNDEFINED && b != UNDEFINED){
        data[a][b] = weight;
        data[b][a] = weight;
        ++edgeCount;
      }
    }catch(...){
      cerr << "FAILED TO ADD EDGE " << a << " " << b << endl;
    }
  }
  void addEdge(const edge& e){
    addEdge(e.a, e.b, e.weight);
  }

  unordered_set<nodeIdx> getCloud(nodeIdx source, unsigned int maxResult){
    typedef pair<nodeIdx, float> halfEdge;
    pQueue<nodeIdx, float> pq;
    unordered_set<nodeIdx> visited, abstracts(maxResult);
    pq.push(source, 0);
    visited.insert(source);
    if(isAbstract(source)) abstracts.insert(source);
    while(!pq.empty() && abstracts.size() < maxResult){
      halfEdge cEdge = pq.pop();
      visited.insert(cEdge.first);
      if(isAbstract(cEdge.first)) abstracts.insert(cEdge.first);
      for(const halfEdge& nEdge : data.at(cEdge.first)){
        if(visited.find(nEdge.first) == visited.end())
          pq.push(nEdge.first, cEdge.second + nEdge.second);
      }
    }
    return abstracts;
  }

  // DEPRICATED
  unordered_set<nodeIdx> getCloud(const vector<nodeIdx> path, unsigned int cloudSetN, unsigned int cloudSetC, unsigned int cloudSetK){
    unordered_set<nodeIdx> res;
    for(unsigned int i = 0; i < path.size(); ++i){
      nodeIdx currNode = path[i];

      if(isAbstract(currNode)){ // get cloud set N

        unordered_set<nodeIdx> set; // doing this so we don't get too much overlap issues
        pQueue<nodeIdx,float> pQ;
        pQ.push(currNode, 0);
        while(!pQ.empty() && set.size() < cloudSetN){
          pair<nodeIdx, float> cPair = pQ.pop();
          nodeIdx cIdx = cPair.first;
          float cWeight = cPair.second;
          set.insert(cIdx);
          for(pair<nodeIdx, float> e : data[cIdx]){
            // If we have yet to add the node to the neighborhood
            if(isAbstract(e.first) && set.find(e.first) == set.end())
              pQ.push(e.first, e.second + cWeight);
          }
        }
        res.insert(set.begin(), set.end());

      } else { // get cloud set K

        // check set is either all the synonyms for a umls term, or the keyword itself
        list<nodeIdx> checkSet;
        if(isUmls(currNode)){
          for(pair<nodeIdx, float> e : data[currNode]){
            if(isKeyword(e.first)) checkSet.push_back(e.first);
          }
        } else {
          checkSet.push_back(currNode);
        }

        pQueue<nodeIdx, float> pQ;
        for(nodeIdx c : checkSet){
          for(pair<nodeIdx, float> e : data[c]){
            if(isAbstract(e.first)) pQ.push(e.first, e.second);
          }
        }
        unsigned int addCount = 0;
        while(!pQ.empty() && addCount < cloudSetK){
          res.insert(pQ.pop().first);
          addCount++;
        }

      }
      //if there are two adjacent keywords
      // get cloud set C
      if(i+1 < path.size() &&
          !isKeyword(currNode) &&
          !isKeyword(path[i+1])){

        nodeIdx nextNode = path[i+1];
        pQueue<nodeIdx, float> pQ;
        for(pair<nodeIdx, float> e : data[currNode]){
          if(isAbstract(e.first) &&
              (data[nextNode].find(e.first) != data[nextNode].end())){
            pQ.push(e.first, (e.second + data[nextNode].at(e.first)) / 2);
          }
        }
        unsigned int addCount = 0;
        while(!pQ.empty() && addCount < cloudSetC){
          res.insert(pQ.pop().first);
          addCount++;
        }

      }
    }
    return res;
  }

private:

  unordered_set<nodeIdx> abstractIdx, umlsIdx;
  unordered_map<nodeIdx, unordered_map<nodeIdx, float>> data;
  unsigned long long edgeCount;

  bool isAbstract(nodeIdx n){
    return abstractIdx.find(n) != abstractIdx.end();
  }
  bool isUmls(nodeIdx n){
    return umlsIdx.find(n) != umlsIdx.end();
  }
  bool isKeyword(nodeIdx n){
    return not (isAbstract(n) || isUmls(n));
  }
};
  //unordered_set<nodeIdx> getAbstractCloud(const path& p, unsigned int size) const{
    //if(verbose){
      //cout << "Fetching cloud for path:";
      //for(nodeIdx n : p.nodes)
        //cout << n << "-";
      //cout << endl;
    //}
  //}
