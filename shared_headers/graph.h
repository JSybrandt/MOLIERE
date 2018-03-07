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
#include<functional>

#include"util.h"
#include"pQueue.h"
#include"parallelFileOp.h"
#include"parallelEdgeListLoad.h"

#include<omp.h>

using namespace std;

class Graph{
public:
  Graph(){};
  Graph(const string& path, const unordered_set<nodeIdx>& subset){
    list<edge> edgeList;
    fastLoadEdgeList(path, edgeList, subset);
    vector<edge> edgeVec;
    edgeVec.reserve(edgeList.size());
    for(auto& e : edgeList){
      edgeVec.emplace_back(move(e));
    }

    edgeCount = edgeVec.size();

    #pragma omp parallel
    {
      unordered_map<nodeIdx, unordered_map<nodeIdx, float>> localGraphData;
      #pragma omp for
      for(size_t i = 0; i < edgeVec.size(); ++i){
        const edge& e = edgeVec[i];
        localGraphData[e.a][e.b] = e.weight;
        localGraphData[e.b][e.a] = e.weight;
      }
      #pragma omp critical
      {
        for(auto& pair : localGraphData){
          data[pair.first].insert(
            pair.second.begin(),
            pair.second.end()
          );
        }
      }
    }
  }

  size_t numNodes() const { return data.size();}
  size_t numEdges() const { return edgeCount;}
  void addEdges(const list<edge>& edges){
    for(const edge& e : edges){
      addEdge(e.a, e.b, e.weight);
    }
  }
  void addEdge(nodeIdx a, nodeIdx b, float weight){
    //if(weight < 0)
      //throw runtime_error("Invalid edge");
    if(a != UNDEFINED && b != UNDEFINED){
      data[a][b] = weight;
      data[b][a] = weight;
      ++edgeCount;
    }
  }

  unordered_set<nodeIdx> getSelectNearby(nodeIdx source,
                                         size_t maxResult,
                                         function<bool(nodeIdx)> filter) const{
    typedef pair<nodeIdx, float> halfEdge;
    pQueue<nodeIdx, float> pq;
    unordered_set<nodeIdx> visited, result(maxResult);
    pq.push(source, 0);
    visited.insert(source);
    if(filter(source)) result.insert(source);
    while(!pq.empty() && result.size() < maxResult){
      halfEdge cEdge = pq.pop();
      visited.insert(cEdge.first);
      if(filter(cEdge.first)) result.insert(cEdge.first);
      for(const halfEdge& nEdge : data.at(cEdge.first)){
        if(visited.find(nEdge.first) == visited.end())
          pq.push(nEdge.first, cEdge.second + nEdge.second);
      }
    }
    return result;
  }

  //bool connected(nodeIdx a, nodeIdx b) const {
    //unordered_map<nodeIdx, nodeIdx> node2set;
    //vector<nodeIdx> nodes;
    //nodes.reserve(data.size());
    //for(const auto& p : data)
      //nodes.push_back(p.first);

    ////initial set
    //#pragma omp parallel for
    //for(size_t i = 0; i < nodes.size(); ++i){
      //const nodeIdx node = nodes[i];
      //const unordered_map<nodeIdx, float>& edges = data.at(node);
      //nodeIdx minId = node;
      //for(const auto& e : edges)
        //minId = min(e.first, minId);
      //#pragma omp critical
      //node2set.emplace(node, minId);
    //}

    //bool anyChanges = true;
    //while(anyChanges){
      //anyChanges = false;
      //if(node2set[a] == node2set[b]){
        //return true;
      //}
      //#pragma omp parallel
      //{
        //unordered_map<nodeIdx, nodeIdx> localUpdates;
        //#pragma omp for
        //for(size_t i = 0; i < nodes.size(); ++i){
          //const nodeIdx node = nodes[i];
          //const unordered_map<nodeIdx, float>& edges = data.at(node);
          //nodeIdx minId = node2set.at(node);
          //for(const auto& e : edges)
            //if(node2set.find(e.first) != node2set.end())
              //minId = min(node2set.at(e.first), minId);
          //if(minId != node2set.at(node)){
            //localUpdates[node] = minId;
          //}
        //}
        //#pragma omp critical
        //{
          //if(localUpdates.size() > 0){
            //anyChanges = true;
            //node2set.insert(localUpdates.begin(), localUpdates.end());
          //}
        //}
      //}
    //}
    //return false;
  //}

  unordered_set<nodeIdx> getNeighbors(nodeIdx source, size_t n) const{
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


  vector<nodeIdx> getShortestPath(nodeIdx source, nodeIdx target) const{
    vector<nodeIdx> res;
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
      while(!backTraversal.empty()){
        res.push_back(backTraversal.top());
        backTraversal.pop();
      }
    }
    return res;
  }

  list<edge> toEdgeList() const{
    list<edge> res;
    for(const auto & node2Negh : data){
      const nodeIdx& i = node2Negh.first;
      for(const auto & pair : node2Negh.second){
        if(pair.first > i){
          edge e;
          e.a = i; e.b = pair.first; e.weight = pair.second;
          res.push_back(e);
        }
      }
    }
    return res;
  }

  friend ostream& operator<<(ostream& out, const Graph& g);

  unordered_set<nodeIdx> getNodes() const {
    unordered_set<nodeIdx> res;
    res.reserve(data.size());
    for(const auto & p : data){
      res.insert(p.first);
    }
    return res;
  }
private:
  typedef pair<nodeIdx, float> halfEdge; // half edge only has one node

  unordered_map<nodeIdx, unordered_map<nodeIdx, float>> data;
  size_t edgeCount;
};

ostream& operator<<(ostream& out, const Graph& g){
  for(const auto & pair : g.data){
    for(const auto & edge : pair.second){
      out << pair.first << " " << edge.first << " " << edge.second << endl;
    }
  }
  return out;
}
