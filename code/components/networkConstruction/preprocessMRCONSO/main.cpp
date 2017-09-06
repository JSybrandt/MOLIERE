#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<fstream>
#include<sstream>
#include"cmdln.h"

using namespace std;

typedef unordered_map<string, unordered_set<string>> AtomDict;

bool verbose;

string processString(string in){
    in.erase(remove_if(in.begin(), in.end(), [](char c){return ispunct(c);}), in.end());
    in.erase(unique(in.begin(), in.end(),
               [](char l, char r){return (l==r) && (l == ' ');}),
               in.end());
    in.erase(in.begin(),
               find_if(in.begin(),in.end(),
                       not1(ptr_fun<int,int>(isspace)))
              );
    in.erase(find_if(in.rbegin(), in.rend(), not1(ptr_fun<int,int>(isspace))).base(),in.end());
    transform(in.begin(), in.end(), in.begin(), [](char c){return tolower(c);});
    transform(in.begin(), in.end(), in.begin(), [](char c){return (c==' ' ? '_':c);});
    return in;
}

AtomDict getCuidAtoms(string umlsHome){
  static const string MRCONSO_PATH = "/META/MRCONSO.RRF";
  static const char MRCONSO_DELIM = '|';
  static const unsigned int CUID_COL_IDX = 0;
  static const unsigned int ATOM_COL_IDX = 14;
  AtomDict res;
  fstream mrconso(umlsHome + MRCONSO_PATH, ios::in);
  string line;
  while(getline(mrconso, line)){
    unsigned int colCount = 0;
    string cuid, atom, token;
    stringstream ss(line);

    while(getline(ss, token, MRCONSO_DELIM)){
      if(colCount == CUID_COL_IDX)
        cuid = token;
      if(colCount == ATOM_COL_IDX)
        atom = token;
      colCount ++;
    }
    atom = processString(atom);
    res[cuid].insert(atom);
  }
  mrconso.close();
  return res;
}

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("umlsHome", 'u', "Directory of UMLS data, need MRSONSO file.", true);
  p.add<string>("outFile", 'o', "Output of preprocessed MRCONSO file.", true);
  p.add("verbose", 'v', "Output debug info.");

  p.parse_check(argc, argv);
  string umlsHomeDir = p.get<string>("umlsHome");
  string outPath = p.get<string>("outFile");
  ::verbose = p.exist("verbose");

  AtomDict cuid2Atoms = getCuidAtoms(umlsHomeDir);

  if(::verbose){
    cout << "Found " << cuid2Atoms.size() << " cuid."<<endl;
    int count = 0;
    for(auto pair : cuid2Atoms){
      count ++;
      if(count > 10) break;
      cout << pair.first << endl;
      for(auto atom : pair.second){
        cout << "\t" << atom << endl;
      }
    }
  }

  fstream outFile(outPath, ios::out);
  for(auto pair : cuid2Atoms){
    outFile << pair.first << " ";
    for(string atom : pair.second){
      outFile << atom << " ";
    }
    outFile << endl;
  }

  outFile.close();

  return 0;
}
