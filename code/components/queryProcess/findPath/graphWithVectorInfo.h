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
typedef unordered_map<nodeIdx, vector<float>> VecDict;

class invalidEdge: public exception {};
class invalidQuery: public exception {};

class graph{
public:
  graph(const VecDict & vecData, const list<edge>& edges): vecData(vecData), edgeCount(0){
    for(const edge& e : edges){
      addEdge(e.a, e.b, e.weight);
    }
  }
  unsigned int numNodes() const { return data.size();}
  unsigned int numEdges() const { return edgeCount;}
  void addEdge(nodeIdx a, nodeIdx b, float weight){
    if(weight < 0)
      throw invalidEdge();
    if(a != UNDEFINED && b != UNDEFINED){
      data[a][b] = weight;
      data[b][a] = weight;
      ++edgeCount;
    }
  }


  vector<nodeIdx> getShortestPath(nodeIdx source, nodeIdx target){
    pQueue<nodeIdx, float> queue;
    unordered_set<nodeIdx> completed;
    queue.push(source, 0);
    bool targetFound = false;
    vector<nodeIdx> path;
    const vector<float>& targetVec = safeGetVec(target);
    float maxDist = distL2(vector<float>(targetVec.size(), -1.0f),
                         vector<float>(targetVec.size(), 1.0f));
    backPointers[source] = source;
#pragma omp parallel
    {
#pragma omp single
    {
      while(!targetFound && !queue.empty()){
        pair<nodeIdx, float> currData;
        currData = queue.pop();
        completed.insert(currData.first);

        for(pair<nodeIdx, float> edge : data[currData.first]){
#pragma omp task firstprivate(edge)
          {
            if(completed.find(edge.first) == completed.end()){
              vector<float> vec;
#pragma omp critical (safeVec)
              vec = safeGetVec(edge.first);
              float d = distL2(vec, targetVec) / maxDist;
#pragma omp critical (bkPtr)
              backPointers[edge.first] = currData.first;
#pragma omp critical (queue)
              queue.push(edge.first, edge.second + currData.second + d);
              if(currData.first == target){
                targetFound = true;
              }
            }
          }
        }
#pragma omp taskwait
      }
    }
    }
    nodeIdx currIdx = target;
    nodeIdx nextIdx = backPointers[currIdx];
    while(currIdx != nextIdx){
      path.push_back(currIdx);
      currIdx = backPointers[currIdx];
      nextIdx = backPointers[currIdx];
    }
    path.push_back(nextIdx);
    return path;
  }
private:

  unsigned int getExpectedVectorLength() const{
    return vecData.begin()->second.size();
  }

  const vector<float>& createApproxVec(nodeIdx idx) {
    if(approxVecs.find(idx) == approxVecs.end()){
      auto d = data.find(idx);
      if(d == data.end())
        throw invalidQuery();
      const unordered_map<nodeIdx, float>& edges = d->second;
      vector<float> res(getExpectedVectorLength(), 0.0f);
      float count = 0;
      for(auto& e : edges){
        auto vec = vecData.find(e.first);
        if(vec != vecData.end()){
          res += vec->second;
          count += 1;
        }
      }
      res /= count;
      approxVecs[idx] = res;
    }
    return approxVecs[idx];
  }

  const vector<float>& safeGetVec(nodeIdx idx){
    auto data = vecData.find(idx);
    if(data == vecData.end()){
      data = approxVecs.find(idx);
      if(data == approxVecs.end()){
        approxVecs[idx] = createApproxVec(idx);
        data = approxVecs.find(idx);
      }
    }
    return data->second;
  }

  unordered_map<nodeIdx, unordered_map<nodeIdx, float>> data;
  unordered_map<nodeIdx, nodeIdx> backPointers;
  const VecDict & vecData;
  VecDict approxVecs;
  unsigned long long edgeCount;
};
