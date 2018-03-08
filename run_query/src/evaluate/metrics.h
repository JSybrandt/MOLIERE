#pragma once

#include<vector>
using std::vector;

#include<unordered_map>
using std::unordered_map;
using std::pair;

#include<map>
using std::map;

#include<cmath>
using std::min;

#include<NetworKit/graph/Graph.h>
#include<NetworKit/centrality/Centrality.h>
#include<NetworKit/centrality/Betweenness.h>
#include<NetworKit/centrality/EigenvectorCentrality.h>
#include<NetworKit/global/ClusteringCoefficient.h>
#include<NetworKit/community/PLP.h>
#include<NetworKit/community/Modularity.h>

#include"util.h"
#include"topic_model.h"
#include"exactNNN.h"

class Metric{
public:
  Metric(const pair<string, string>& queryWords_,
         const vector<Topic>& topicModel_,
         const vector<vector<float>>& topicCentroids_,
         const unordered_map<string, vector<float>>& word2vec_
  ): queryWords(queryWords_),
     topicModel(topicModel_),
     topicCentroids(topicCentroids_),
     word2vec(word2vec_){}
  virtual ~Metric() = default;

  virtual float calculate() = 0;

protected:
  const pair<string, string>& queryWords;
  const vector<Topic>& topicModel;
  const vector<vector<float>>& topicCentroids;
  const unordered_map<string, vector<float>>& word2vec;


  float topicSim(const string& word, const Topic& topic){
    const vector<float>& selectVec = word2vec.at(word);
    float totalCount = 0;
    float totalSum = 0;

    for(auto & wordProb : topic){
      auto vec = word2vec.find(wordProb.first);
      if(vec != word2vec.end()){
        totalCount += wordProb.second;
        totalSum += wordProb.second * cosSim(selectVec, vec->second);
      }
    }
    return totalSum / totalCount;
  }
};

class PerTopicMetric : public Metric{
public:
  PerTopicMetric(
    const pair<string, string>& queryWords_,
    const vector<Topic>& topicModel_,
    const vector<vector<float>>& topicCentroids_,
    const unordered_map<string, vector<float>>& word2vec_
  ): Metric(queryWords_, topicModel_, topicCentroids_, word2vec_){}

  virtual ~PerTopicMetric() = default;

  // returns score for topic i
  virtual float getTopicScore(size_t i){
    if(scores.find(i) == scores.end()){
      scores[i] = calculateTopicScore(i);
    }
    return scores[i];
  };

  // returns topic idx, score in order highest to lowest
  vector<pair<size_t, float>> getBestTopics(){
    // ordered map used for tree sort
    map<float, vector<size_t>> score2topic;
    for(size_t i = 0; i < topicModel.size(); ++i){
      score2topic[getTopicScore(i)].push_back(i);
    }

    vector<pair<size_t, float>> res;
    res.reserve(topicModel.size());

    //from largest score to smallest
    for(auto p = score2topic.rbegin(); p != score2topic.rend(); ++p){
      for(size_t t : p->second){
        res.push_back({t, p->first});
      }
    }
    return res;
  }
protected:
  virtual float calculateTopicScore(size_t) = 0;
  unordered_map<size_t, float> scores;
};


class L2Metric : public Metric{
public:
  L2Metric(
    const pair<string, string>& queryWords_,
    const vector<Topic>& topicModel_,
    const vector<vector<float>>& topicCentroids_,
    const unordered_map<string, vector<float>>& word2vec_
  ): Metric(queryWords_, topicModel_, topicCentroids_, word2vec_){}

  virtual ~L2Metric() = default;

  virtual float calculate(){
    return distL2(word2vec.at(queryWords.first),
                  word2vec.at(queryWords.second));
  }
};


class BestCentrL2Metric : public PerTopicMetric{
public:
  BestCentrL2Metric(
    const pair<string, string>& queryWords_,
    const vector<Topic>& topicModel_,
    const vector<vector<float>>& topicCentroids_,
    const unordered_map<string, vector<float>>& word2vec_
  ): PerTopicMetric(queryWords_, topicModel_, topicCentroids_, word2vec_)
  {
    midPoint = word2vec.at(queryWords.first) + word2vec.at(queryWords.second);
    midPoint /= 2;
    diffNorm = magnitude(word2vec.at(queryWords.first)
                         - word2vec.at(queryWords.second));
    diffNorm /= 2;
  }

  virtual ~BestCentrL2Metric() = default;

  virtual float calculate(){
    return getBestTopics()[0].second;
  }

protected:
  virtual float calculateTopicScore(size_t i){
    return 1 - (magnitude(topicCentroids[i] - midPoint) / diffNorm);
  }

private:
  vector<float> midPoint;
  float diffNorm;
};


class CSimMetric : public Metric{
public:
  CSimMetric(
    const pair<string, string>& queryWords_,
    const vector<Topic>& topicModel_,
    const vector<vector<float>>& topicCentroids_,
    const unordered_map<string, vector<float>>& word2vec_
  ): Metric(queryWords_, topicModel_, topicCentroids_, word2vec_){}

  virtual ~CSimMetric() = default;

  virtual float calculate(){
    return cosSim(word2vec.at(queryWords.first),
                  word2vec.at(queryWords.second));
  }
};

class BestCentrCSimMetric : public PerTopicMetric{
public:
  BestCentrCSimMetric(
    const pair<string, string>& queryWords_,
    const vector<Topic>& topicModel_,
    const vector<vector<float>>& topicCentroids_,
    const unordered_map<string, vector<float>>& word2vec_
  ): PerTopicMetric(queryWords_, topicModel_, topicCentroids_, word2vec_){}

  virtual ~BestCentrCSimMetric() = default;

  virtual float calculate(){
    return getBestTopics()[0].second;
  }

protected:
  virtual float calculateTopicScore(size_t i){
    float c1 = cosSim(word2vec.at(queryWords.first), topicCentroids[i]);
    float c2 = cosSim(word2vec.at(queryWords.second), topicCentroids[i]);
    return (c1 + c2) / 2;
  }
};


class TopicCorrMetric: public Metric{
public:
  TopicCorrMetric(
    const pair<string, string>& queryWords_,
    const vector<Topic>& topicModel_,
    const vector<vector<float>>& topicCentroids_,
    const unordered_map<string, vector<float>>& word2vec_
  ): Metric(queryWords_, topicModel_, topicCentroids_, word2vec_){}

  virtual ~TopicCorrMetric() = default;

  virtual float calculate(){
    vector<float> v1, v2;
    v1.reserve(topicModel.size());
    v2.reserve(topicModel.size());
    for(const Topic& t : topicModel){
      v1.push_back(topicSim(queryWords.first, t));
      v2.push_back(topicSim(queryWords.second, t));
    }
    return cosSim(v1, v2);
  }
};

class BestTopicPerWordMetric: public PerTopicMetric{
public:
  BestTopicPerWordMetric(
    const pair<string, string>& queryWords_,
    const vector<Topic>& topicModel_,
    const vector<vector<float>>& topicCentroids_,
    const unordered_map<string, vector<float>>& word2vec_
  ): PerTopicMetric(queryWords_, topicModel_, topicCentroids_, word2vec_){}

  virtual ~BestTopicPerWordMetric() = default;

  virtual float calculate(){
    return getBestTopics()[0].second;
  }
protected:
  virtual float calculateTopicScore(size_t i){
    const Topic& t = topicModel[i];
    return min(topicSim(queryWords.first, t),
               topicSim(queryWords.second, t));
  }
};

struct TopicNetworkMetricData{
  TopicNetworkMetricData(
    const pair<string, string>& queryWords_,
    const vector<Topic>& topicModel_,
    const vector<vector<float>>& topicCentroids_,
    const unordered_map<string, vector<float>>& word2vec_
  ): queryWords(queryWords_),
     topicModel(topicModel_),
     topicCentroids(topicCentroids_),
     word2vec(word2vec_)
  {
    size_t numNN = 3;
    vector<vector<float>> networkPoints;
    networkPoints.reserve(topicCentroids.size() + 2);
    networkPoints.push_back(word2vec.at(queryWords.first));
    networkPoints.push_back(word2vec.at(queryWords.second));
    networkPoints.insert(networkPoints.end(),
                         topicCentroids.begin(),
                         topicCentroids.end());
    Graph graph = createNNN(networkPoints, numNN);

    //get shortest path between the two query words
    queryWordPath = graph.getShortestPath(0, 1);
    while(queryWordPath.size() == 0){
      numNN += 1;
      graph = createNNN(networkPoints, numNN);
      queryWordPath = graph.getShortestPath(0, 1);
    }

    //our graph is loaded, need to convert to nk graph
    for(size_t i = 0; i < graph.numNodes(); ++i){
      nkGraph.addNode();
    }
    for(edge& e : graph.toEdgeList()){
      nkGraph.addEdge(e.a, e.b, e.weight);
    }
  }

  const pair<string, string>& queryWords;
  const vector<Topic>& topicModel;
  const vector<vector<float>>& topicCentroids;
  const unordered_map<string, vector<float>>& word2vec;

  NetworKit::Graph nkGraph;
  vector<nodeIdx> queryWordPath;
};

class TopicNetworkMetric: public Metric{
public:
  TopicNetworkMetric(TopicNetworkMetricData& d):
    Metric(d.queryWords, d.topicModel, d.topicCentroids, d.word2vec),
    nkGraph(d.nkGraph), queryWordPath(d.queryWordPath){}

  virtual ~TopicNetworkMetric() = default;
protected:
  NetworKit::Graph& nkGraph;
  const vector<nodeIdx>& queryWordPath;
};

class TopicWalkBtwnMetric : public TopicNetworkMetric{
public:
  TopicWalkBtwnMetric(TopicNetworkMetricData& d):TopicNetworkMetric(d){}

  virtual ~TopicWalkBtwnMetric() = default;

  virtual float calculate(){
    NetworKit::Betweenness btwn(nkGraph);
    btwn.run();
    float totalBtwn = 0;
    for(nodeIdx n : queryWordPath){
      totalBtwn += btwn.score(n);
    }
    return totalBtwn / queryWordPath.size();
  }
};

class TopicWalkEigenMetric : public TopicNetworkMetric{
public:
  TopicWalkEigenMetric(TopicNetworkMetricData& d):TopicNetworkMetric(d){}

  virtual ~TopicWalkEigenMetric() = default;

  virtual float calculate(){
    NetworKit::EigenvectorCentrality eigen(nkGraph);
    eigen.run();
    float totalEigen = 0;
    for(nodeIdx n : queryWordPath){
      totalEigen += eigen.score(n);
    }
    return totalEigen / queryWordPath.size();
  }
};

class TopicNetCCoefMetric : public TopicNetworkMetric{
public:
  TopicNetCCoefMetric(TopicNetworkMetricData& d):TopicNetworkMetric(d){}

  virtual ~TopicNetCCoefMetric() = default;

  virtual float calculate(){
    return NetworKit::ClusteringCoefficient::exactGlobal(nkGraph);
  }
};

class TopicNetModMetric : public TopicNetworkMetric{
public:
  TopicNetModMetric(TopicNetworkMetricData& d):TopicNetworkMetric(d){}

  virtual ~TopicNetModMetric() = default;

  virtual float calculate(){
    NetworKit::Modularity mod;
    NetworKit::PLP partition(nkGraph);
    partition.run();
    return mod.getQuality(partition.getPartition(), nkGraph);
  }
};
