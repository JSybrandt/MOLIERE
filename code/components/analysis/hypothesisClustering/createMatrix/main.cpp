#include<iostream>
#include<fstream>
#include<unordered_map>
#include<unordered_set>
#include<sstream>
#include<string>
#include<exception>
#include<fstream>
#include<vector>
#include"cmdln.h"
#include"pgmb_io.hpp"

using namespace std;

typedef unsigned int nodeIdx;
const nodeIdx UNDEFINED = numeric_limits<nodeIdx>::max();

struct InvalidPathException: public exception{
  string msg;
  InvalidPathException(string msg):msg(msg){}
  virtual const char* what() const throw(){
    return this->msg.c_str();
  }
};

struct DijkResult{
  unordered_set<nodeIdx> cloud;
  nodeIdx start;
  string startLbl;
  nodeIdx end;
  string endLbl;
  vector<nodeIdx> path;
  float pathWeight;
  DijkResult(string pathData, string abstractCloud, vector<string>& labels){
    // Loading Path Data
    stringstream pathStream(pathData);
    string token;
    // get start
    pathStream >> token;
    if (token != "Start:")
      throw InvalidPathException("Missing Start");
    pathStream >> this->start;
    if(this->start >= labels.size())
      throw InvalidPathException("Start index greater than label vector size.");
    this->startLbl = labels[this->start];
    // get end
    pathStream >> token;
    if (token != "End:")
      throw InvalidPathException("Missing End");
    pathStream >> this->end;
    if(this->end >= labels.size())
      throw InvalidPathException("End greater than label vector size.");
    this->endLbl = labels[this->end];
    // get path & weight
    pathStream >> token;
    if (token != "Path:")
      throw InvalidPathException("Missing Path");
    pathStream >> token;
    while(token != "Weight:" && pathStream){
      path.push_back(stoi(token));
      pathStream >> token;
    }
    if (token != "Weight:")
      throw InvalidPathException("Missing Weight");
    pathStream >> this->pathWeight;

    // Loading Cloud
    nodeIdx cloudIdx;
    stringstream cloudStream(abstractCloud);
    while(cloudStream >> cloudIdx){
      cloud.insert(cloudIdx);
    }
  }

  float calcSim(const DijkResult& other) const{
    int overlap = 0;
    for(nodeIdx cloudIdx : this->cloud){
      if(other.cloud.find(cloudIdx) != other.cloud.end())
        overlap++;
    }
    return 2 * float(overlap) / float(this->cloud.size() + other.cloud.size());
  }

  string toString() const{
    stringstream s;
    s << this->startLbl << "---" << this->endLbl;
    return s.str();
  }
};

unsigned char float2uchar(float val, float maxVal = 1.0f){
  val /= maxVal;
  val *= numeric_limits<unsigned char>::max();
  return (unsigned char) val;
}

int main(int argc, char ** argv){
   cmdline::parser p;

  p.add<string>("dijkstraFile", 'd', "input file from dijkstra results", true, "");
  p.add<string>("labelFile", 'l', "input file of graph labels", true, "");
  p.add<string>("outMatFile", 'o', "output pbm file.", true, "");
  p.add<string>("outHypoNamesFile", 'O', "output text file with hypo names.", true, "");
  p.add("verbose", 'v', "outputs debug information");

  p.parse_check(argc, argv);

  string dijkPath =  p.get<string>("dijkstraFile");
  string labelPath =  p.get<string>("labelFile");
  string outMatrixPath =  p.get<string>("outMatFile");
  string outHypoNamesPath =  p.get<string>("outHypoNamesFile");
  bool verbose = p.exist("verbose");

  if(verbose){cout<<"Loading"<<labelPath<<endl;}
  fstream labelFile(labelPath, ios::in);
  vector<string> labels;
  string tmp;
  while(labelFile >> tmp){
    labels.push_back(tmp);
  }
  if(verbose){cout<<"Closing"<<labelPath<<endl;}
  labelFile.close();


  if(verbose){cout<<"Loading"<<dijkPath<<endl;}
  fstream dijkFile(dijkPath, ios::in);

  vector<DijkResult> paths;

  while(dijkFile){
    string pathData, cloudData;
    getline(dijkFile, pathData);
    getline(dijkFile, cloudData);
    if(pathData == "" || cloudData == "")
      break;
    paths.push_back(DijkResult(pathData, cloudData, labels));
  }
  if(verbose){cout<<"Found "<<paths.size() <<" paths in "<< dijkPath<<endl;}

  if(verbose){cout<<"Closing"<<dijkPath<<endl;}
  dijkFile.close();

  if(verbose){cout<<"Calculating Sims "<<endl;}

  unsigned char * simData = new unsigned char[paths.size() * paths.size()];

#pragma omp parallel for schedule(dynamic)
  for(unsigned int i=0; i < paths.size(); ++i)
    for(unsigned int j=i; j < paths.size(); ++j)
    {
      unsigned char sim = numeric_limits<unsigned char>::max();
      if(i != j){
        DijkResult& path1 = paths[i];
        DijkResult& path2 = paths[j];
        sim = float2uchar(path1.calcSim(path2));
        simData[i*paths.size() + j] = sim;
      }
      simData[j*paths.size() + i] = sim;
      //if(verbose){cout<<"Sim:"<<i<<","<<j<<"="<<(int)sim<<endl;}
    }

  if(verbose){cout<<"Writing Sims to "<<outMatrixPath<<endl;}
  pgmb_write(outMatrixPath, paths.size(), paths.size(), simData);
  if(verbose){cout<<"Cleaning up Sims"<<endl;}
  delete[] simData;

  if(verbose){cout<<"Writing hypo names to "<<outHypoNamesPath<<endl;}
  fstream hypoNamesFile(outHypoNamesPath, ios::out);

  for(DijkResult path : paths){
    hypoNamesFile << path.toString() << endl;
  }

  if(verbose){cout<<"Closing "<<outHypoNamesPath<<endl;}
  hypoNamesFile.close();

  if(verbose){cout<<"DONE! "<<endl;}

}
