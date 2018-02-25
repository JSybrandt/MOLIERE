#pragma once
#include<algorithm>
#include<vector>
#include<map>
#include<iostream>

using std::cout;
using std::endl;

using std::vector;
using std::make_heap;
using std::push_heap;
using std::pop_heap;
using std::pair;

template<class K, class V>
struct TupleCompare{
  bool operator()(pair<K, V> a, pair<K, V> b){
    return a.second < b.second;
  }
};

template<class K, class V>
class FixedSizeHeap{
private:
  typedef pair<K, V> tuple;
  typedef vector<tuple> heap;

public:
  FixedSizeHeap(unsigned int maxSize): maxSize(maxSize), isHeap(false) {
    if(maxSize == 0){
      throw std::runtime_error("FixedSizeHeap: Size must be positive.");
    }
  };
  void insert(K k, V v){
    if(size() == maxSize && !isHeap){
      make_heap(data.begin(), data.end(), TupleCompare<K, V>());
      isHeap = true;
    }
    if(size() < maxSize || v < data[0].second){
      data.push_back(tuple(k, v));
      if(isHeap)
        push_heap(data.begin(), data.end(), TupleCompare<K, V>());
    }
    while(isHeap && size() > maxSize){
      pop_heap(data.begin(), data.end(), TupleCompare<K, V>());
      data.pop_back();
    }
  }
  unsigned int size(){
    return data.size();
  }

  const heap& getHeap() const { return data; }

private:
  bool isHeap;
  unsigned int maxSize;
  heap data;

};
