#pragma once
#include"util.h"
#include"parallelFileOp.h"
#include"files.h"

#include<unordered_map>
#include<list>
#include<string>

using std::runtime_error;
using std::list;
using std::vector;
using std::string;
using std::pair;


class VectorManager{
public:
  unordered_map<string, vector<float>> getVectors(const unordered_set<string>& labels) const{
    list<pair<string, vector<float>>> w2vList;
    unordered_map<char, unordered_set<string>> tag2lbls;
    for(const string& lbl : labels)
      tag2lbls[getTag(lbl)].insert(lbl);
    for(const auto& p : tag2lbls){
      char tag = p.first;
      if(! hasTag(tag)) throw runtime_error("Attempted to get vector for unregistered tag.");
      const unordered_set<string>& localLables = p.second;

      auto inSet = [&localLables](const string& line) -> bool {
        stringstream ss(line);
        string tmp;
        ss >> tmp;
        return localLables.find(tmp) != localLables.end();
      };

      fastProcFile<pair<string, vector<float>>>(tag2path.at(tag), w2vList,
                                                line2vec, inSet);
    }
    unordered_map<string, vector<float>> res;
    for(auto& l : w2vList)
      res.emplace(move(l));
    return res;
  }

  void registerTag(const string& path, char tag = '_'){
    // Tells us where to look given a vector's tag.
    // The '_' tag represents no tag, used for ngrams.
    // Tags must be capital letters.
    if(tag != '_' && (tag < 'A' || tag >'Z'))
      throw runtime_error("Tag must be a capital letter or '_'");
    if(!isFile(path))
      throw runtime_error("Tagged vector file must exist");
    tag2path[tag] = path;
  }
  bool hasTag(char tag) const {
    return tag2path.find(tag) != tag2path.end();
  }
private:
  unordered_map<char, string> tag2path;
  char getTag(const string& lbl) const{
    char c = lbl[0];
    if(c >= 'A' && c <= 'Z')
      return c;
    return '_';
  }
};

