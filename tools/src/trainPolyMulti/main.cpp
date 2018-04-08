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

#include"eval.h"
#include"paramPicker.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;

const size_t STEPS_BTWN_SHRINK = 100;


//gets the min and range for each value in SCORE_NAMES
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


int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("positives", 'p', "list of positive example eval files", true);
  p.add<string>("negatives", 'n', "list of negative example eval files", true);
  p.add<string>("output", 'o', "output param file", true);
  p.add<size_t>("num-trials", 't', "Number of bb trials", false, 100000);
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string positivesPath =  p.get<string>("positives");
  string negativesPath =  p.get<string>("negatives");
  string outPath =  p.get<string>("output");
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
    auto e2s = [&param](const Eval& e) -> float {
      return e.polyMulti(param);
    };
    float roc = calcROC(evals, e2s);
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
}
