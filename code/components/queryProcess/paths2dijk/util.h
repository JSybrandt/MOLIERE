#pragma once

#include<vector>
#include<iostream>
#include<sstream>
#include<exception>
#include<cmath>

using std::vector;
using std::cerr;
using std::endl;
using std::stringstream;
using std::exception;
using std::string;

void string2vec(const string& line, string& label, vector<float>& vec){
  stringstream ss(line);
  ss >> label;
  float temp;
  while(ss >> temp){ vec.push_back(temp);}
}

class vectorLengthNotEqual: public exception {};

float dist(const vector<float>& ptA, const vector<float>& ptB){
  if(ptA.size() != ptB.size()){
    cerr << ptA.size() << " != " << ptB.size() << endl;
    throw vectorLengthNotEqual();
  }

  float res = 0;
  for(unsigned int i = 0; i < ptA.size(); ++i){
    res += pow(ptA[i]-ptB[i], 2);
  }
  return sqrt(res);
}

union charBuff{
  unsigned int i;
  float f;
  char buff[4];
};

void operator += (vector<float>& a, const vector<float>& b){
  if(a.size() != b.size())
    throw vectorLengthNotEqual();
  for(unsigned int i = 0; i < a.size(); ++i){
    a[i] += b[i];
  }
}

void operator /= (vector<float>& a, float b){
  for(unsigned int i = 0; i < a.size(); ++i){
    a[i] /= b;
  }
}
