#include<iostream>
#include<unordered_map>
#include<map>
#include<unordered_set>
#include<string>
#include<vector>
#include<queue>
#include<limits>
#include<fstream>
#include<stack>
#include<sstream>
#include<exception>
#include<list>
#include<random>

#include"cmdln.h"
#include"files.h"
#include"util.h"
#include"parallelFileOp.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

//min and range (-1 - 1)
const pair<float, float> COEF_RANGE = {-1, 2};
const pair<float, float> POW_RANGE = {-1, 2};

const size_t STEPS_BTWN_SHRINK = 100;

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
      const string& name = scoreNames[i];
      if(scores.find(name) != scores.end()){
        res[i] = scores.at(name);
      } else {
        throw runtime_error("Failed to find score:" + name);
      }
    }
    return res;
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

unordered_map<string, pair<float, float>> getRanges(const vector<Eval>& evals){
  vector<float> maxes(SCORE_NAMES.size(), -numeric_limits<float>::infinity());
  vector<float> mins(SCORE_NAMES.size(), numeric_limits<float>::infinity());
  #pragma omp parallel
  {
    vector<float> localMaxes(SCORE_NAMES.size(), -numeric_limits<float>::infinity());
    vector<float> localMins(SCORE_NAMES.size(), numeric_limits<float>::infinity());
    #pragma omp for
    for(size_t i = 0; i < evals.size(); ++i){
      const vector<float>& vals = evals[i].toVec(SCORE_NAMES);
      for(size_t j = 0; j < vals.size(); ++j){
        localMaxes[j] = max(vals[j], localMaxes[j]);
        localMins[j] = min(vals[j], localMins[j]);
      }
    }
    #pragma omp critical
    {
      for(size_t j = 0; j < localMaxes.size(); ++j){
        maxes[j] = max(maxes[j], localMaxes[j]);
        mins[j] = min(mins[j], localMins[j]);
      }
    }
  }
  unordered_map<string, pair<float, float>> res;
  for(size_t i = 0; i < maxes.size(); ++i){
    res[SCORE_NAMES[i]] = {mins[i], maxes[i]-mins[i]};
  }
  return res;
}

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
              const unordered_map<string, pair<float, float>>& name2CoefPow,
              vector<pair<float, float>>* rocRet = nullptr){
  // start by making a helper array
  vector<pair<float, bool>> score2label(evals.size());
  #pragma omp parallel for
  for(size_t i = 0; i < evals.size(); ++i){
    const auto& e = evals[i];
    score2label[i] = {e.polyMulti(name2CoefPow), e.getLabel()};
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

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("positives", 'p', "list of positive example eval files", true);
  p.add<string>("negatives", 'n', "list of negative example eval files", true);
  p.add<string>("output", 'o', "output param file", true);
  p.add<string>("roc-plot-out", 'r', "output roc results", false, "");
  p.add<size_t>("num-trials", 't', "Number of bb trials", false, 100000);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string positivesPath =  p.get<string>("positives");
  string negativesPath =  p.get<string>("negatives");
  string outPath =  p.get<string>("output");
  string rocPlotPath =  p.get<string>("roc-plot-out");
  size_t numTrials = p.get<size_t>("num-trials");
  ::verbose = p.exist("verbose");

  list<Eval> evalList;
  auto loadPos = [](const string& path) -> Eval{
    return Eval(path, true);
  };
  auto loadNeg = [](const string& path) -> Eval{
    return Eval(path, false);
  };

  vout  << "Loading positive eval files" << endl;
  fastProcFile<Eval>(positivesPath, evalList, loadPos);
  vout << "Loading negative eval files" << endl;
  fastProcFile<Eval>(negativesPath, evalList, loadNeg);

  vout << "List -> vec" << endl;
  vector<Eval> evals;
  evals.reserve(evalList.size());
  for(Eval& e : evalList){
    evals.emplace_back(move(e));
  }

  vout << "Getting maxes" << endl;
  unordered_map<string, pair<float, float>> ranges = getRanges(evals);

  vout << "Scaling evals" << endl;
  #pragma omp parallel for
  for(size_t i = 0; i < evals.size(); ++i){
    evals[i].scale(ranges);
  }

  vout << "Testing ROC" << endl;
  float bestScore = 0;
  unordered_map<string, pair<float,float>> bestParam;
  ParamPicker picker;
  size_t iterSinceBest = 0;
  for(size_t i = 0 ; i < numTrials; ++i){
    const auto param = picker.getRandParam();
    float roc = calcROC(evals, param);
    ++iterSinceBest;

    if(roc > bestScore){
      bestScore = roc;
      bestParam = param;
      picker.setBestParam(bestParam, 1.0f);
      iterSinceBest = 0;
      if(verbose){
        vout << "Score: " << bestScore << "\tIt: " << i << endl;
        for(const auto & p : param){
          vout << "\t" << p.first << ":\t"
               << p.second.first << "\t" << p.second.second << endl;
        }
      }
    } else if(iterSinceBest%STEPS_BTWN_SHRINK== 0){
      float del = 1.0f / (iterSinceBest / float(STEPS_BTWN_SHRINK));
      vout << "Shrinking: " << del << "\tIt:" << i << endl;
      picker.setBestParam(bestParam, del);
    }
  }

  vout << "Writing Param File" << endl;
  fstream outFile(outPath, ios::out);
  for(const string& name : SCORE_NAMES){
    float coef = bestParam[name].first;
    float power = bestParam[name].second;
    float min = ranges[name].first;
    float range = ranges[name].second;
    vout << name << " "
         << coef << " "
         << power << " "
         << min << " "
         << range << endl;
    outFile << name << " "
            << coef << " "
            << power << " "
            << min << " "
            << range << endl;
  }
  outFile.close();

  if(rocPlotPath != ""){
    vector<pair<float, float>> xys;
    calcROC(evals, bestParam, &xys);
    fstream outFile(rocPlotPath, ios::out);
    for(pair<float, float> xy : xys){
      outFile << xy.first << " " << xy.second << endl;
    }
    outFile.close();
  }
}
