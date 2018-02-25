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

#include"util.h"
#include"pQueue.h"

#include<omp.h>

using namespace std;

class Graph{
public:
  unsigned int numNodes() const { return data.size();}
  unsigned int numEdges() const { return edgeCount;}
  void addEdge(nodeIdx a, nodeIdx b, float weight){
    //if(weight < 0)
      //throw runtime_error("Invalid Edge");
    if(a != UNDEFINED && b != UNDEFINED){
      data[a][b] = weight;
      data[b][a] = weight;
      ++edgeCount;
    }
  }

  struct Path{
    vector<nodeIdx> path;
    float weight;
  };

  struct Edge{
    nodeIdx a, b;
    float w;
  };

  unordered_set<nodeIdx> getNeighbors(nodeIdx source, unsigned int n){
    unordered_set<nodeIdx> visited;
    pQueue<nodeIdx, float> pq;
    pq.push(source, 0);
    // n+1 so that we don't return source
    while(!pq.empty() && visited.size() < n+1){
      halfEdge cEdge = pq.pop();
      visited.insert(cEdge.first);
      for(const halfEdge & nEdge : data.at(cEdge.first)){
        if(visited.find(nEdge.first) == visited.end())
          pq.push(nEdge.first, cEdge.second + nEdge.second);
      }
    }
    visited.erase(source);
    return visited;
  }

  Path getShortestPath(nodeIdx source, nodeIdx target) const{
    // first, we are going to tighten edges
    pQueue<nodeIdx, float> pq;
    unordered_map<nodeIdx, float> finalDists;
    pq.push(source, 0);
    while(!pq.empty() && finalDists.find(target) == finalDists.end()){
      halfEdge cEdge = pq.pop();
      finalDists[cEdge.first] = cEdge.second; // record the tight edge
      for(const halfEdge & nEdge : data.at(cEdge.first)){
        if(finalDists.find(nEdge.first) == finalDists.end()){
          pq.push(nEdge.first, nEdge.second + cEdge.second);
        }
      }
    }

    // if we found the goal
    if(finalDists.find(target) != finalDists.end()){
      //we traverse only tight edges dfs from s to t
      stack<nodeIdx> stk;
      stk.push(source);
      unordered_map<nodeIdx, nodeIdx> backPointers;
      nodeIdx cNode;
      while((cNode = stk.top()) != target){
        bool foundTightEdge = false; // we have to back track if we get stuck
        for(const halfEdge & nEdge : data.at(cNode)){
          const nodeIdx & nNode = nEdge.first;
          // no self loop
          if(cNode != nNode
              // was visited
              && finalDists.find(nEdge.first) != finalDists.end()
              // is tight edge
              && abs(finalDists[cNode] + data.at(cNode).at(nNode) - finalDists[nNode]) < EPS
              // not a backpointer already
              && backPointers.find(nNode) == backPointers.end()){
                stk.push(nNode);
                backPointers[nNode] = cNode;
                foundTightEdge = true;
          }
        }
        if(!foundTightEdge){
          stk.pop(); // backtrack
        }
      }

      stack<nodeIdx> backTraversal;
      backTraversal.push(target);
      while(backTraversal.top() != source){
        backTraversal.push(backPointers[backTraversal.top()]);
      }
      Path res;
      while(!backTraversal.empty()){
        res.path.push_back(backTraversal.top());
        backTraversal.pop();
      }
      res.weight = finalDists[target];
      return res;
    } else {
      Path res;
      res.weight = -1;
      return res;
    }
  }

  list<Edge> toEdgeList() const{
    list<Edge> res;
    for(const auto & node2Negh : data){
      const nodeIdx& i = node2Negh.first;
      for(const auto & pair : node2Negh.second){
        if(pair.first > i){
          Edge e;
          e.a = i; e.b = pair.first; e.w = pair.second;
          res.push_back(e);
        }
      }
    }
    return res;
  }

  friend ostream& operator<<(ostream& out, const Graph& g);

  unsigned int getNodeCount(){return data.size();}
private:
  typedef pair<nodeIdx, float> halfEdge; // half edge only has one node

  unordered_map<nodeIdx, unordered_map<nodeIdx, float>> data;
  unsigned long long edgeCount;
};

ostream& operator<<(ostream& out, const Graph& g){
  for(const auto & pair : g.data){
    for(const auto & edge : pair.second){
      out << pair.first << " " << edge.first << " " << edge.second << endl;
    }
  }
  return out;
}

ostream& operator<<(ostream& out, const Graph::Path& p){
  out << p.weight << " : ";
  for(nodeIdx n : p.path){
    out << n << " ";
  }
  return out;
}

ostream& operator<<(ostream& out, const Graph::Edge& e){
  out << e.a << " " << e.b << " " << e.w;
  return out;
}
