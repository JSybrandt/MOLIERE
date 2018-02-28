#pragma once

#include<vector>
#include<string>
#include<sstream>
#include<ostream>
#include<unordered_map>

using std::vector;
using std::string;
using std::stringstream;
using std::ostream;
using std::unordered_map;


class Bow{
public:
  Bow():name(""),totalCount(0){};
  Bow(const string& name_, const unordered_map<string, size_t> counts_):name(name_),counts(counts_),totalCount(counts_.size()){}
  Bow(const string& record, size_t ignoreCount = 1, bool firstIsName = true):name(""),totalCount(0){
    add(record, ignoreCount, firstIsName);
  }
  void add(const Bow& other){
    for(const auto& pair : other.counts){
      if(this->counts.find(pair.first) == this->counts.end()){
        this->counts[pair.first] = pair.second;
      } else {
        this->counts[pair.first] += pair.second;
      }
      totalCount += pair.second;
    }
  }
  void add(const string& record, size_t ignoreCount = 0, bool firstIsName = false){
    stringstream ss(record);
    bool first = true;
    string tmp;
    while(ss >> tmp){
      if(first && firstIsName) name = tmp;
      first = false;
      if(ignoreCount > 0){
        --ignoreCount;
      } else {
        if(counts.find(tmp) == counts.end()) counts[tmp] = 1;
        else counts[tmp] += 1;
        totalCount += 1;
      }
    }
  }
  size_t getTotalCount() const{
    return totalCount;
  }
  const unordered_map<string, size_t>& getCounts() const{
    return counts;
  }
  size_t getCount(const string& word) const{
    if(counts.find(word) == counts.end()){
      return 0;
    } else {
      return counts.at(word);
    }
  }
  Bow toOccurances() const{
    Bow res;
    for(auto & pair: this->counts){
      res.counts[pair.first] = 1;
    }
    return res;
  }
  string getName() const{
    return name;
  }
private:
  string name;
  unordered_map<string, size_t> counts;
  size_t totalCount;
  friend ostream& operator<<(ostream&, const Bow&);
};

ostream& operator<<(ostream& out, const Bow& bag){
  out << bag.name;
  bool first = true;
  for(const auto & pair : bag.counts){
    if(!first)
      out << " ";
    first = false;
    out << pair.first << " " << pair.second;
  }
  return out;
}
