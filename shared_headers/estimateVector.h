#pragma once

#include<vectorManager.h>
#include<labelManager.h>
#include<util.h>
#include<graph.h>

// This function is for finding a node's vector
// given that the node does not contain an embedding,
// but its neighbors do.
vector<float> estimateVector(const string& lbl, const string& graphPath, VectorManager& vm, LabelManager& lm){
  nodeIdx source = lm[lbl];

  // get all 1st order neighbors
  Graph graph(graphPath, {source});
  unordered_set<nodeIdx> neighbors = graph.getNodes();
  unordered_set<string> neighLbls;
  neighLbls.reserve(neighbors.size());
  for(nodeIdx i : neighbors)
    neighLbls.insert(lm[i]);

  unordered_map<string, vector<float>> w2v = vm.getVectors(neighLbls);

  if(w2v.find(lbl) != w2v.end()){
    // dummy, we already had it
    return w2v.at(lbl);
  }

  // get average of found vectors
  float count = 0;
  vector<float> res;
  for(const auto& p : w2v){
    const auto& vec = p.second;
    if(count == 0)
      res = vec;
    else
      res += vec;
    count += 1;
  }
  res /= count;
  return res;
}
