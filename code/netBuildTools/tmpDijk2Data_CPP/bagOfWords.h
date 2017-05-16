#pragma once

/*
 * Reusabel class for converting text to bags of words
 */

#include<string>
#include<fstream>
#include<ostream>
#include<unordered_map>
#include<sstream>

using namespace std;

class bagOfWords{
  unordered_map<string,unsigned int> bag;
public:
  bagOfWords(){}
  bagOfWords(string text){
    stringstream ss(text);
    string word;
    while(ss >> word){
      if(bag.find(word) == bag.end()){
        bag[word] = 0;
      }
      bag[word]++;
    }
  }

  string toString(){
    stringstream ss;
    bool first = true;
    for(auto pair : bag){
      if(!first) ss << " ";
      ss << pair.first << " " << pair.second;
      first = false;
    }
    return ss.str();
  }

  ostream& operator<<(ostream& out){
    out << toString();
    return out;
  }

  fstream& operator<<(fstream& out){
    out << toString();
    return out;
  }
};
