#pragma once
#include<iostream>
#include"../fixedSizeHeap.h"
using namespace std;
int main(){
  FixedSizeHeap<string, float> heap(5);
  heap.insert("1", 1);
  heap.insert("2", 2);
  heap.insert("3", 3);
  heap.insert("4", 4);
  heap.insert("5", 5);
  heap.insert("6", 6);
  heap.insert("6", 6);
  heap.insert("6", 6);
  heap.insert("6", 6);
  heap.insert("6", 6);
  heap.insert("6", 6);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  heap.insert("1", 1);
  for(auto pair : heap.getHeap()){
    cout << pair.first << " " << pair.second << endl;
  }
  return 0;
}
