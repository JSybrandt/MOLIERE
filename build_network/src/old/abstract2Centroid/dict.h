#pragma once

//This file can take *.VEC and make an unordered map

#include<unordered_map>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include"vec.h"

using std::unordered_map;
using std::fstream;
using std::string;
using std::stringstream;

class Dict {
private:

	unordered_map<string, Vec> data;
  int vecSize;
  int numRecords;

public:

	Dict(string file);
	const Vec getVec(string word);
  const bool contains(string word);
  const int size(){return data.size();}
};
