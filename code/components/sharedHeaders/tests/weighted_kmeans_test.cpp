#include<iostream>
#include<vector>
#include<map>
#include<ctime>
#include"../weighted_kmeans.h"

using namespace std;

int main(){
  srand(time(NULL));
  vector<pair<vector<float>,float>> data = {
    make_pair<vector<float>,float>({1, 2}, 1),
    make_pair<vector<float>,float>({2, 3}, 10),
    make_pair<vector<float>,float>({3, 2}, 1),
    make_pair<vector<float>,float>({-1, -1}, 1),
    make_pair<vector<float>,float>({-2, -2}, 10),
    make_pair<vector<float>,float>({-2, -3}, 1),
    make_pair<vector<float>,float>({-1, -2}, 1),
    make_pair<vector<float>,float>({10, -10}, 100),
    make_pair<vector<float>,float>({-1, 2}, 1),
    make_pair<vector<float>,float>({-2, 2}, 1),
    make_pair<vector<float>,float>({-1, 3}, 1),
    make_pair<vector<float>,float>({-3, 2}, 10)
  };

  unsigned int k = 4;

  KMeansRes res = weightedKmeans(data, k);

  cout << "Centroids:" << endl;
  for(unsigned int i = 0; i < k; ++i){
    cout << i << " :";
    for(float f : res.centroids[i]){
      cout << " " << f;
    }
    cout << endl;
  }

  cout << "Labels:" << endl;
  for(unsigned int i = 0; i < data.size(); ++i){
    cout << res.labels[i] << " :";
    for(float f : data[i].first){
      cout << " " << f;
    }
    cout << endl;
  }

  return 0;
}
