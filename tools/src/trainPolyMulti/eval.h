#pragma once

#include<string>
using std::string;

#include"files.h"

#include<fstream>
using std::fstream;
using std::ios;

#include<unordered_map>
using std::unordered_map;
using std::pair;

#include<cmath>

#include<vector>
using std::vector;

#include<algorithm>
using std::sort;

#include<limits>
using std::numeric_limits;

#include<functional>
using std::function;



const vector<string> SCORE_NAMES = {
  "TopicNetCCoef",
  "TopicWalkBtwn",
  "TopicCorr",
  "BestTopicPerWord",
  "BestCentrL2",
  "L2"
};



class Eval{
public:
  Eval(const string& path, bool isPos):isPositive(isPos){
    if(!isFile(path))
      throw runtime_error("Not a file: " + path);
    fstream fin(path, ios::in);
    string line, metricName;
    float score;
    while(getline(fin, line)){
      stringstream ss(line);
      ss >> metricName >> score;
      scores[metricName] = score;
    }
    fin.close();
  }

  vector<float> toVec(const vector<string>& scoreNames) const{
    vector<float> res(scoreNames.size(), 0.0f);
    for(size_t i = 0; i < scoreNames.size(); ++i){
      res[i] = getScore(scoreNames[i]);
    }
    return res;
  }

  float getScore(const string& name) const {
      if(scores.find(name) != scores.end()){
        return scores.at(name);
      }
      throw runtime_error("Failed to find score:" + name);
  }

  void scale(const unordered_map<string, pair<float, float>>& ranges){
    for(const auto p: ranges){
      const string& name = p.first;
      float minScore = p.second.first;
      float scoreRange = p.second.second;
      if(scoreRange == 0){
        throw runtime_error("Cannot divide by zero. Don't put it in the scale map.");
      }
      if(scores.find(name) != scores.end()){
        scores[name] = (scores[name] - minScore) / scoreRange;
      } else {
        throw runtime_error("Failed to find score:" + name);
      }
    }
  }

  float polyMulti(const unordered_map<string, pair<float, float>> name2CoefPow) const {
    float res = 0;
    for(const auto p: name2CoefPow){
      const string& name = p.first;
      pair<float, float> coefPow = p.second;
      if(scores.find(name) != scores.end()){
        res += coefPow.first * pow(scores.at(name), coefPow.second);
      } else {
        throw runtime_error("Failed to find score:" + name);
      }
    }
    return res;
  }

  bool getLabel() const {
    return isPositive;
  }

private:
  // want alphabetical order
  unordered_map<string, float> scores;
  bool isPositive;
};


//precondition: sorted in x
float getAUC(const vector<pair<float, float>>& xyPts){
  float area = 0;
  #pragma omp parallel
  {
    float localArea = 0;
    #pragma omp for
    for(size_t i = 1; i < xyPts.size(); ++i){
      // trapezoids
      const pair<float, float> & a = xyPts[i-1];
      const pair<float, float> & b = xyPts[i];
      float w = b.first - a.first;
      float h = (b.second + a.second) / 2;
      localArea += w * h;
    }
    #pragma omp critical
    area += localArea;
  }
  return area;
}

float calcROC(const vector<Eval>& evals,
              const function<float(const Eval&)>& eval2score,
              vector<pair<float, float>>* rocRet = nullptr){
  // start by making a helper array
  vector<pair<float, bool>> score2label(evals.size());
  #pragma omp parallel for
  for(size_t i = 0; i < evals.size(); ++i){
    const auto& e = evals[i];
    score2label[i] = {eval2score(e), e.getLabel()};
  }
  // sort in reverse order by first, true breaks ties
  static auto cmp = [](const pair<float, bool>& a, const pair<float, bool>& b) -> bool {
    if(a.first == b.first){
      if(a.second == b.second)
        return false;
      return b.second;
    }
    return a.first > b.first;
  };
  sort(score2label.begin(), score2label.end(), cmp);

  float conPos = 0, conNeg = 0;

  #pragma omp parallel
  {
    float localTP = 0, localTN = 0;
    #pragma omp for
    for(size_t i = 0; i < score2label.size(); ++i){
      if(score2label[i].second){
        ++localTP;
      }else{
        ++localTN;
      }
    }
    #pragma omp critical
    {
      conPos += localTP;
      conNeg += localTN;
    }
  }

  //TODO: someday I might parallelize this, but I'm lazy
  vector<pair<float, float>> fracts;
  fracts.reserve(score2label.size());
  float tp = 0, fp = 0, lastScore = -numeric_limits<float>::infinity();
  for(const auto& p : score2label){
    float score = p.first;
    bool lbl = p.second;
    if(score !=  lastScore){
      fracts.emplace_back(tp/conPos, fp/conNeg);
      lastScore = score;
    }
    if(lbl){
      ++tp;
    } else {
      ++fp;
    }
  }
  // adds (1, 1)
  fracts.emplace_back(tp/conPos, fp/conNeg);

  // optional return value
  if(rocRet){
    *rocRet = fracts;
  }


  return getAUC(fracts);
}
