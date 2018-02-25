#pragma once

// requires linking flann
#include<flann/flann.hpp>
#include<flann/algorithms/dist.h>
#include<vector>
#include<list>
#include<unordered_map>

#include"util.h"
#include"graph.h"

using namespace std;

namespace flann{

  // WARNING,  this is going to assume normalized vectors
  template<class T>
  struct CosineDistance
  {
      // typedef bool is_kdtree_distance;

      typedef T ElementType;
      typedef typename Accumulator<T>::Type ResultType;

      template <typename Iterator1, typename Iterator2>
      ResultType operator()(Iterator1 a, Iterator2 b, size_t size, ResultType worst_dist = -1) const
      {
          ResultType result = ResultType();
          Iterator1 last = a + size;

          ResultType l1 = ResultType();
          ResultType l2 = ResultType();
          while (a < last) {
              result += (ResultType)(*a * *b);
              l1 += *a * *a;
              l2 += *b * *b;
              ++a;
              ++b;
          }
          result /= (sqrt(l1) + sqrt(l2));
          return -result + 1;
      }

  };
}

typedef vector<float> Point;

graph createNNN(vector<Point>& points, unsigned int numNN, bool l2 = true, unsigned int numTrees = 16){
  unsigned int numVecs = points.size();
  if(numVecs == 0)
    return graph();
  unsigned int vectorSize = points[0].size(); // first vec size
  for(const Point& point : points)
    if(point.size() != vectorSize)
      throw runtime_error("Not all vectors are the same size.");
  float* rawData = new float[vectorSize * numVecs];
  for(unsigned int vecIdx = 0 ; vecIdx < numVecs; ++vecIdx){
    Point& point = points[vecIdx];
    float mag = magnitude(point);
    unsigned int startIdx = vecIdx * vectorSize;
    for(unsigned int dataIdx = 0; dataIdx < vectorSize; ++dataIdx){
        rawData[startIdx + dataIdx] = point[dataIdx]; // normalize as we load
    }
  }
  flann::Matrix<float> flannData(rawData, numVecs, vectorSize);
  vector<vector<int> > indicies;
  vector<vector<float> > dists;
  if(l2){
    flann::Index<flann::L2<float> > index(flannData, flann::KDTreeIndexParams(numTrees));
    index.buildIndex();
    flann::SearchParams params(0);
    params.cores = 0; //automatic core selection
    index.knnSearch(flannData, indicies, dists, numNN,  params);
  } else {
    flann::Index<flann::CosineDistance<float> > index(flannData, flann::LinearIndexParams());
    index.buildIndex();
    flann::SearchParams params(0);
    params.cores = 0; //automatic core selection
    index.knnSearch(flannData, indicies, dists, numNN,  params);
  }

  graph g;
  for(unsigned int nodeIdx =0; nodeIdx < numVecs; ++nodeIdx){
    for(unsigned int neighborIdx : indicies[nodeIdx]){
      if(nodeIdx != neighborIdx){
        g.addEdge(nodeIdx, neighborIdx, dists[nodeIdx][neighborIdx]);
      }
    }
  }
  return g;
}



