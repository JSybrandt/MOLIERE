/*
 * Given the directory for the UMLS metathesurus data
 * generate a network of keywords
 *
 * Note: datafile ref: https://www.ncbi.nlm.nih.gov/books/NBK9685/
 */
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<utility>
#include<fstream>
#include<sstream>
#include<limits>
#include<algorithm>
#include<cmath>
#include<cstdio>
#include<stdlib.h>
#include"cmdln.h"

using namespace std;

typedef std::pair<unsigned int, unsigned int> edge;
struct edgeHash {
    inline std::size_t operator()(const edge & v) const {
        return v.first * 15485863 + v.second * 15485867;
   }
};
typedef std::unordered_set<edge, edgeHash> edgeSet;


/*
 * We aren't actually going to use this, instead we can map CUI to term in a different prog
// Load from MRCONSO file
// column labels described here:
// https://www.ncbi.nlm.nih.gov/books/NBK9685/table/ch03.T.concept_names_and_sources_file_mr/?report=objectonly
struct KeywordData{
  const static int ID_COL_IDX = 0;
  const static int STR_COL_IDX = 14;
  KeywordData(string data){
    vector<string> record = getRecord(data);
    cui = record[ID_COL_IDX];
    text = record[STR_COL_IDX];
  }
  string cui;
  string text;
};
*/

// Load from MRREL file
// column labels described here:
// https://www.ncbi.nlm.nih.gov/books/NBK9685/table/ch03.T.related_concepts_file_mrrel_rrf/?report=objectonly
// relationship types here: https://www.nlm.nih.gov/research/umls/knowledge_sources/metathesaurus/release/abbreviations.html
struct RelationshipData{
  const static int IDA_COL_IDX = 0;
  const static int IDB_COL_IDX = 4;
  const static int REL_TYPE_COL_INX = 3;
  // literally no ducking clue why I need this.
  static constexpr float STRONG = 0.1f;
  static constexpr float MEDIUM = 0.5f;
  static constexpr float WEAK = 1.0f;
  static unordered_map<string, float> edgeWeights;

  // The following are all possible relationship types
  // going to make similar relationship types equal weight
  // edgeWeights will be from 0 to 1 where 0 is "stronger"
  // because our network is undirected, I only include one size of
  // all semetric links
  static unordered_map<string, float> makeWeights(){
    unordered_map<string, float> res;
    res["AQ"] = WEAK; // allowed qualifier
    res["CHD"] = MEDIUM; // child
    // res["DEL"] = 1; // deleted
    res["PAR"] = MEDIUM; // parent
    res["QB"] = WEAK; // can be qualified by
    res["RB"] = MEDIUM; // broader relationship
    res["RL"] = STRONG; // 'alike' or similar
    res["RN"] = MEDIUM; // narrower
    res["RO"] = WEAK; // OTHER
    res["RQ"] = STRONG; // related or synonimous
    res["RU"] = WEAK; // unspecified
    res["SIB"] = MEDIUM; // sibling
    res["SY"] = STRONG; // source asserted synonymy
    // res["XR"] = 1; // notRelated
    // res[""] = 0; // empty
    return res;
  }

  RelationshipData(string data){
    vector<string> record = getRecord(data);
    cuiA = record[IDA_COL_IDX];
    cuiB = record[IDB_COL_IDX];
    weight = 999;
    string relType = record[REL_TYPE_COL_INX];
    if(edgeWeights.find(relType) == edgeWeights.end()){
      cout<<"Failed to find \"" << relType << "\" in the weight dict" << endl;
      isValid = false;
    }
    else{
      weight = edgeWeights[relType];
      isValid = true;
    }
  }

  string cuiA;
  string cuiB;
  float weight;
  bool isValid;

  vector<string> getRecord(string line, char delim = '|'){
    stringstream s(line);
    string tmp;
    vector<string> res;
    while(getline(s, tmp, delim)){
      res.push_back(tmp);
    }
    return res;
  }
};

unordered_map<string, float> RelationshipData::edgeWeights = RelationshipData::makeWeights();

inline void checkAddLabel(unordered_map<string, unsigned int> & label2int, string& label, fstream& labelFile){
  static unsigned int numVals = 0;
  if(label2int.find(label) == label2int.end()){
    label2int[label] = numVals;
    numVals++;
    labelFile << label << endl;
  }
}

int main(int argc, char** argv){

  cmdline::parser p;

  p.add<string>("umlsDir",    'd', "input directory of umls", true, "");
  p.add<string>("outputFile", 'o', "output file name, will result in .edges and .labels", true, "");
  p.add("verbose", 'v', "outputs debug information");
  p.add<float>("edgeWeights", 'w', "the maximal value of resulting edge weights", false, 1);

  p.parse_check(argc, argv);

  string umlsDirPath =  p.get<string>("umlsDir");
  string outputPath =  p.get<string>("outputFile");
  bool verbose = p.exist("verbose");
  float edgeWeights = p.get<float>("edgeWeights");

  if(verbose) cout << "Started" << endl;

  // string mrconsoPath = umlsDirPath+"/META/MRCONSO.RRF";
  string mrrelPath = umlsDirPath+"/META/MRREL.RRF";
  string outGraphPath = outputPath + ".edges";
  string outLabelPath = outputPath + ".labels";

  if(verbose) cout << "Opening output files" << endl;
  fstream outGraphFile(outGraphPath, ios::out);
  fstream outLabelFile(outLabelPath, ios::out);

  if(verbose) cout << "READING: " << mrrelPath << endl;

  unordered_map<string, unsigned int> label2int;

  unordered_map<edge, float, edgeHash> edge2Weight;

  string line;
  fstream mrrelFile(mrrelPath, ios::in);
  while(getline(mrrelFile, line)){
    if(verbose) cout << line << endl;
    RelationshipData data(line);
    if(data.isValid){
      if(verbose){
        cout << "Relationship:" << data.cuiA << " " << data.cuiB << " " << data.weight << endl;
      }

      checkAddLabel(label2int, data.cuiA, outLabelFile);
      checkAddLabel(label2int, data.cuiB, outLabelFile);

      unsigned int s = min(label2int[data.cuiA], label2int[data.cuiB]);
      unsigned int t = max(label2int[data.cuiA], label2int[data.cuiB]);

      edge e(s,t);
      if(s != t){
        if(edge2Weight.find(e) == edge2Weight.end()){
          if(verbose){
            cout << "Discovered " << e.first << " " << e.second
                 << " " << data.weight << endl;
          }
          edge2Weight[e] = data.weight;
        } else { // only remeber the best link
          if(verbose){
            cout << "Updating weight of " << data.cuiA << " "
                 << data.cuiB << " to " << data.weight << endl;
          }
          edge2Weight[e] = min(data.weight, edge2Weight[e]);
        }
      }
    } else if(verbose){
      cout << "Found invalid edge.\t" << data.cuiA << "-" << data.cuiB << endl;
    }
  }

  for(auto pair : edge2Weight){
    // edge.first edge.second weight
    if(verbose){
      cout << "OUTPUTTING: " << pair.first.first << " " << pair.first.second << " " << pair.second * edgeWeights << endl;

    }
    outGraphFile << pair.first.first << " " << pair.first.second << " " << pair.second * edgeWeights << endl;
  }
  mrrelFile.close();
  outGraphFile.close();
  outLabelFile.close();

  if(verbose) cout << "Done, cleaning up" << endl;
}
