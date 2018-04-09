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

using namespace std;

bool verbose = false;
#define vout if(::verbose) cout

typedef tuple<float, float, float, float> FourFloats;

unordered_map<string, FourFloats> getHParam(const string& path){
  unordered_map<string, FourFloats> param;
  fstream fin(path, ios::in);
  string name;
  float coef, expo, minVal, range;

  // load from file
  while(fin >> name >> coef >> expo >> minVal >> range){
    param[name] = {coef, expo, minVal, range};
  }
  fin.close();
  return param;
}

void writeRes(ostream& out, const string& name, float auc, const vector<pair<float,float>> xys){
  out << name << " " << auc << endl;
  for(auto xy : xys)
    out << xy.first << " ";
  out << endl;
  for(auto xy : xys)
    out << xy.second << " ";
  out << endl;
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("positives", 'p', "list of positive example eval files", true);
  p.add<string>("negatives", 'n', "list of negative example eval files", true);
  p.add<string>("hyperparam", 'y', "input hyperparameter file", true);
  p.add<string>("output", 'o', "output param file", true);
  p.add("force-above-midline", 'm', "always outputs so value is greater than midline");
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string positivesPath =  p.get<string>("positives");
  string negativesPath =  p.get<string>("negatives");
  string hyperParamPath =  p.get<string>("hyperparam");
  string outPath =  p.get<string>("output");
  bool forceAboveMidline = p.exist("force-above-midline");
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

	vout << "Loading hyper param" << endl;
	unordered_map<string, FourFloats> rawParam = getHParam(hyperParamPath);
  unordered_map<string, pair<float, float>> param, ranges;
  for(auto& p : rawParam){
    // coef and pow
    param[p.first] = {get<0>(p.second), get<1>(p.second)};
    // min and range
    ranges[p.first] = {get<2>(p.second), get<3>(p.second)};
  }

  vout << "Scaling" << endl;
  #pragma omp parallel for
  for(size_t i = 0; i < evals.size(); ++i){
    evals[i].scale(ranges);
  }

  vector<pair<float, float>> xys;
  float auc;
  fstream outFile(outPath, ios::out);
  vout << "Writing Poly Multiple data" << endl;
  auto e2c = [&param](const Eval& e) -> float {
    return e.polyMulti(param);
  };
  xys.clear();
  auc = calcROC(evals, e2c, &xys);
  if(auc < 0.5 && forceAboveMidline){
    auto e2c = [&param](const Eval& e) -> float {
      return - e.polyMulti(param);
    };
    xys.clear();
    auc = calcROC(evals, e2c, &xys);
  }
  vout << "AUC:" << auc << endl;
  writeRes(outFile, "PolyMultiple", auc, xys);

  for(const string& name : SCORE_NAMES){
    vout << "Writing " << name << endl;
    auto e2c = [&name](const Eval& e) -> float {
      return e.getScore(name);
    };
    xys.clear();
    auc = calcROC(evals, e2c, &xys);
    if(auc < 0.5 && forceAboveMidline){
      auto e2c = [&name](const Eval& e) -> float {
        return - e.getScore(name);
      };
      xys.clear();
      auc = calcROC(evals, e2c, &xys);
    }
    vout << "AUC:" << auc << endl;
    writeRes(outFile, name, auc, xys);
  }

  outFile.close();


}
