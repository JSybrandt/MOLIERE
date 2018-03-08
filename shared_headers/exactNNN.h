#pragma once

#include<vector>
#include<list>
#include<unordered_map>

#include"util.h"
#include"graph.h"
#include"pQueue.h"

#include<omp.h>

using namespace std;

typedef vector<float> Point;

Graph createNNN(const vector<Point>& points, unsigned int numNN, bool l2 = true){
  unsigned int numVecs = points.size();
  if(numVecs == 0)
    return Graph();
  unsigned int vectorSize = points[0].size(); // first vec size
  for(const Point& point : points)
    if(point.size() != vectorSize)
      throw runtime_error("Not all vectors are the same size.");

  Graph g;
#pragma omp parallel for
  for(unsigned int  i = 0 ; i < numVecs; ++i){
    pQueue<unsigned int, float> pq; //neighbor 2 dist
    for(unsigned int j = 0; j < numVecs; ++j){
      if(j != i){
        float dist;
        if(l2){
          dist = distL2(points[i], points[j]);
        } else {
          dist = -cosSim(points[i], points[j]) + 1;
        }
        pq.push(j, -dist); // push neg dist so higher dist are lower priority
        while(pq.size() > numNN){
          pq.pop();
        }
      }
    }

#pragma omp critical
    while(pq.size() > 0){
      pair<unsigned int, float> edge = pq.pop();
      g.addEdge(i, edge.first, -edge.second); // neg again becuase of above
    }
  }

  return g;
}



