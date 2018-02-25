/*
 * The goal here is to calculate some statistics about each of the predicate TM's that we generate.
 * This means that we need to read a predicate, a TM file, and the synonyms of that word.
 *
 *
 * How hard is it to load this file... thats worth a check
 * It takes less than half a second to load the file, we can do that.
 */

#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<fstream>
#include<sstream>
#include<cmath>
#include<omp.h>
#include<limits>
#include<algorithm>

#include"cmdln.h"
#include"util.h"
#include"topic_model.h"
#include"exactNNN.h"
#include"graph.h"

#include <NetworKit/graph/Graph.h>
#include <NetworKit/centrality/Centrality.h>
#include <NetworKit/centrality/Betweenness.h>
#include <NetworKit/centrality/EigenvectorCentrality.h>
#include <NetworKit/global/ClusteringCoefficient.h>
#include <NetworKit/community/PLP.h>
#include <NetworKit/community/Modularity.h>

using namespace std;

// an array of lists of pairs
typedef vector<pair<string, unsigned int>> Topic;

bool verbose;
#define vout if(::verbose) cout

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("topicModel", 'm', "Topic model from VIEW_FILES", true);
  p.add<string>("ngramVecs", 'n', "ngram vector file", false, "");
  p.add<string>("pmidVecs", 'p', "pmid vector file", false, "");
  p.add<string>("cuidVecs", 'c', "cuid vector file", false, "");
  p.add<string>("sourceLabel", 's', "Source Label", true);
  p.add<string>("targetLabel", 't', "Target Label", true);
  p.add("verbose", 'v', "Output debug info.");
  p.add("euclidian", 'e', "Use euclidian distance instead of cosine similarity.");

  p.parse_check(argc, argv);
  string topicModelPath = p.get<string>("topicModel");
  string ngramVecsPath = p.get<string>("ngramVecs");
  string pmidVecsPath = p.get<string>("pmidVecs");
  string cuidVecsPath = p.get<string>("cuidVecs");
  string sourceLabel = p.get<string>("sourceLabel");
  string targetLabel = p.get<string>("targetLabel");
  ::verbose = p.exist("verbose");
  bool euclidian = p.exist("euclidian");

  vout << "Loading topic model from " << topicModelPath << endl;
  vector<Topic> topics = getTM(topicModelPath);
  vout << "Loaded  " << topics.size() << " topics." << endl;

  vout << "Making GetVector Obj." << endl;
  GetVector getVec(ngramVecsPath, pmidVecsPath, cuidVecsPath);

  vout << "Getting all unique labels" << endl;
  unordered_set<string> labels;
  labels.insert(sourceLabel);
  labels.insert(targetLabel);
  for(const Topic & topic : topics){
    for(const auto & p : topic){
      labels.insert(p.first);
    }
  }

  vout << "Found " << labels.size() << " uniq vectors." << endl;
  vout << "Loading word vectors for each label" << endl;

  unordered_map<string, vector<float>> word2vec = getVec(labels);

  vout << "Found " << word2vec.size() << " word vectors." << endl;

  vout << "Calculating topic centers." << endl;
  vector<vector<float>> dataPoints(topics.size() + 2); // add in s and t
  vector<float> topicScores(topics.size(), 0);

  //#pragma omp parallel for
  for(unsigned int i = 0; i < topics.size(); ++i){
    dataPoints[i] = getCentroid(topics[i], word2vec);
  }

  nodeIdx sourceIdx = topics.size();
  nodeIdx targetIdx = topics.size() + 1;
  dataPoints[sourceIdx] = word2vec[sourceLabel];
  dataPoints[targetIdx] = word2vec[targetLabel];

  unsigned int numNN = 3;
  do{
    vout << "NN = " << numNN << endl;
    Graph graph = createNNN(dataPoints, numNN, euclidian);
    vout << "Getting shortest path" << endl;
    Graph::Path path = graph.getShortestPath(sourceIdx, targetIdx);
    path = graph.getShortestPath(sourceIdx, targetIdx);
    if(!path.path.empty()){
      if(::verbose){
        vout << "Path:";
        for(nodeIdx n : path.path)
          vout << n << " ";
        vout << endl;
      }
      vout << "Getting Centralities" << endl;
      // networkit graph with n nodes, weighted and undirected
      NetworKit::Graph nkGraph(0, true, false);
      for(unsigned int i = 0; i < graph.numNodes(); ++i){
        nkGraph.addNode();
      }
      // add graph to NK representation
      for(edge& e : graph.toEdgeList()){
        if(e.a != e.b){
          nkGraph.addEdge(e.a, e.b, e.weight);
          vout << e << endl;
        }
      }
      NetworKit::Betweenness btwn(nkGraph);
      NetworKit::EigenvectorCentrality eigen(nkGraph);
      vout << "Running..." << endl;
      btwn.run();
      eigen.run();
      float totalBtwn = 0;
      float totalEigen = 0;
      for(nodeIdx n : path.path){
        totalBtwn += btwn.score(n);
        totalEigen += eigen.score(n);
      }
      vout << "Clustering" << endl;
      NetworKit::PLP plp(nkGraph);
      plp.run();
      NetworKit::Modularity mod;
      vout << "Outputting" << endl;
      cout << sourceLabel << " "
           << targetLabel << " "
           << numNN << " "
           << path.weight << " "
           << path.weight / path.path.size() << " "
           << (btwn.score(sourceIdx) + btwn.score(targetIdx))/2 << " "
           << totalBtwn / path.path.size() << " "
           << (eigen.score(sourceIdx) + eigen.score(targetIdx))/2 << " "
           << totalEigen / path.path.size() << " "
           << NetworKit::ClusteringCoefficient::exactGlobal(nkGraph) << " "
           << mod.getQuality(plp.getPartition(), nkGraph) << " "
           << endl;
    break;
    } else
      numNN++;
  } while(true);

  return 0;
}
