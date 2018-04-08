#pragma once

#include<unordered_map>
using std::unordered_map;
using std::pair;

#include<random>
using std::random_device;
using std::mt19937;
using std::uniform_real_distribution;

#include<string>
using std::string;

#include<algorithm>
using std::min;
using std::max;

#include"eval.h"

const pair<float, float> COEF_RANGE = {-1, 2};
const pair<float, float> POW_RANGE = {-1, 2};

class ParamPicker{
public:

  ParamPicker(){
    resetDomain();
  }

  // make ranges centered around interesting point
  void setBestParam(const unordered_map<string, pair<float, float>>& pt, float del){
    auto setWin = [&del](float v, const pair<float, float>& range) -> pair<float, float>{
      float l = max(v - del, range.first);
      float h = min(v + del, range.first + range.second);
      return {l, h-l};
    };
    for(const string& name : SCORE_NAMES){
      coefRanges[name] = setWin(pt.at(name).first, COEF_RANGE);
      powRanges[name] = setWin(pt.at(name).second, POW_RANGE);
    }
  }

  void resetDomain(){
    for(const string& name : SCORE_NAMES){
      coefRanges[name] = COEF_RANGE;
      powRanges[name] = POW_RANGE;
    }
  }

  unordered_map<string, pair<float, float>> getRandParam(){
    static random_device dev;
    static mt19937 gen(dev());
    static uniform_real_distribution<> unitDist(0, 1);
    unordered_map<string, pair<float, float>> res;
    static auto pick = [&unitDist, &gen](pair<float, float> range) -> float {
      return range.first + (range.second * unitDist(gen));
    };
    for(const string& name: SCORE_NAMES){
      res[name] = {pick(coefRanges[name]), pick(powRanges[name])};
    }
    return res;
  }


private:
  unordered_map<string, pair<float, float>> coefRanges, powRanges;
};
