#include<unordered_map>
#include<algorithm>
#include<vector>
#include<limits>
#include<omp.h>
#include"util.h"

using std::unordered_map;
using std::pair;
using std::vector;
using std::numeric_limits;


struct KMeansRes {
  vector<unsigned int> labels;
  vector<vector<float>> centroids;
};
// input is a list of (pt, w) tuples
// output is a list of labels, where label[i] corresponds to (pt_i, w_i)
KMeansRes weightedKmeans(const vector<pair<vector<float>, float>> data,
                                     unsigned int k){
  unsigned int vecSize = data[0].first.size();
  KMeansRes res;

  vector<unsigned int>& labels = res.labels;
  labels = vector<unsigned int>(data.size(), 0);

  auto& centroids = res.centroids;
  centroids = vector<vector<float>>(k, vector<float>(vecSize, 0.0f));

  // initial labels
//#pragma omp parallel for
  for(unsigned int i = 0 ; i < data.size(); ++i){
    labels[i] = rand()%k;
  }
  unsigned int changedLabels = 0;
  float centroidDrift = 0;
  unsigned int iter = 0;
  do{
    ++iter;
    //calc centroids
    vector<vector<float>> newCentroids(k, vector<float>(vecSize, 0.0f));
    vector<float> counts(k, 0.0f);
//#pragma omp parallel for
    for(unsigned int i = 0 ; i < data.size(); ++i){
      const vector<float> & vec = data[i].first;
      const float weight = data[i].second;
      const unsigned int label = labels[i];

//#pragma omp critical(centroids)
      newCentroids[label] += vec * weight;
//#pragma omp critical(counts)
      counts[label] += weight;
    }

    centroidDrift = 0;
//#pragma omp parallel for
    for(unsigned int i = 0 ; i < k; ++i){
      if(counts[i] > 0){
        newCentroids[i] /= counts[i];
        float diff = distL2(centroids[i], newCentroids[i]);
//#pragma omp critical(drift)
        centroidDrift += diff;
      }
    }
    centroids = newCentroids;
    changedLabels = 0;
//#pragma omp parallel for
    for(unsigned int i = 0; i < data.size(); ++i){
      const vector<float>& vec = data[i].first;
      unsigned int & label = labels[i];
      unsigned int newLabel = label;
      float minDist = numeric_limits<float>::infinity();
      for(unsigned int l = 0 ; l < k; ++l){
        float dist = distL2(centroids[l], vec);
        if(minDist > dist){
          minDist = dist;
          newLabel = l;
        }
      }
      if(newLabel != label){
//#pragma omp critical(labels)
        ++changedLabels;
        label = newLabel;
      }
    }
  } while(changedLabels != 0 || centroidDrift > 0.0001 || iter == 1);
  return res;
}

