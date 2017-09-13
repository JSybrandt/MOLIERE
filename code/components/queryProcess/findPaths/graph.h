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

#include"pQueue.h"
#include"util.h"

#include<omp.h>

using namespace std;

typedef unsigned int nodeIdx;
typedef unordered_map<nodeIdx, vector<float>> VecDict;
const nodeIdx UNDEFINED = numeric_limits<nodeIdx>::max();
constexpr unsigned int NUM_BYTE_PER_EDGE = 12;

class invalidEdge: public exception {};
class invalidQuery: public exception {};

struct edge{
  edge(): a(UNDEFINED), b(UNDEFINED), weight(0) {}
  edge(nodeIdx a, nodeIdx b, float w): a(a), b(b), weight(w) {}
  nodeIdx a, b;
  float weight;
};

class graph{
public:
  graph(const VecDict & vecData, const vector<edge>& edges): vecData(vecData), edgeCount(0){
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


  //TODO: impliment a better union-find
  bool pathExists(nodeIdx a, nodeIdx b){
    return true;
    //unordered_set<nodeIdx> completed;
    //stack<nodeIdx> stk;
    //stk.push(a);
    //bool result = false;
    //bool shouldGoOn = true;
    //while(shouldGoOn){
      //nodeIdx currNode;
      //currNode = stk.top(); stk.pop();
      //if(currNode == b){
        //result = true;
        //break;
      //}
      //for(auto edge : data[currNode]){
        //if(completed.find(edge.first) == completed.end()){
          //stk.push(edge.first);
          //if(edge.first == b){
            //result = true;
            //break;
          //}
        //}
      //}
      //shouldGoOn = !stk.empty();
    //}
    //return result;
  }


  vector<nodeIdx> getShortestPath(nodeIdx source, nodeIdx target){
    pQueue<nodeIdx, float> queue;
    unordered_set<nodeIdx> completed;
    queue.push(source, 0);
    bool targetFound = false;
    vector<nodeIdx> path;
    const vector<float>& targetVec = safeGetVec(target);
    float maxDist = dist(vector<float>(targetVec.size(), -1.0f),
                         vector<float>(targetVec.size(), 1.0f));
    bool shouldGoOn = true;
#pragma omp parallel
    {
#pragma omp single
    {
      while(shouldGoOn){
        pair<nodeIdx, float> currData;
#pragma omp critical (queue)
        currData = queue.pop();
#pragma omp critical (out)
        cout << "Viewing: " << currData.first << " : " << currData.second << endl;

#pragma omp critical (completed)
        completed.insert(currData.first);

#pragma omp task firstprivate (currData)
        {
          vector<float> vec;
#pragma omp critical (approxVec)
          vec = safeGetVec(currData.first);
          float d = dist(vec, targetVec) / maxDist;
          for(pair<nodeIdx, float> edge : data[currData.first]){
            float isFresh;
#pragma omp critical (completed)
            isFresh = completed.find(edge.first) == completed.end();
            if(isFresh){
#pragma omp critical (backPointers)
              backPointers[edge.first] = currData.first;
#pragma omp critical (queue)
              queue.push(edge.first, edge.second + currData.second + d);
              if(currData.first == target){
#pragma omp critical (targetFound)
                targetFound = true;
              }
            }
          }
        }
        bool mustWait = false;
#pragma omp critical (queue)
        mustWait = queue.empty();
        if(mustWait){
#pragma omp taskwait
#pragma omp critical (queue)
          if(queue.empty()) shouldGoOn = false;
        }
#pragma omp critical (targetFound)
        if(targetFound) shouldGoOn = false;
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
      cout << "WARNING: CREATING APPROX VEC : " << idx << endl;
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
