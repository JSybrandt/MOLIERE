/*
 * Flann Runner
 *
 * Given a data file, this program will make a knn graph
 *
 *
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
#include"cmdln.h"
#include"dict.h"
#include"vec.h"

using namespace std;

int main(int argc, char** argv){

  cmdline::parser p;

  p.add<string>("dictFile",    'd', "input vector dictionary", true, "");
  p.add<string>("abstractFile", 'a', "input abstract file", true, "");
  p.add<string>("centroidFile", 'c', "output centroid file", true, "");
  p.add("verbose", 'v', "outputs debug information");
  p.add<unsigned int>("vecSize", 's', "dimensionality of vectors", false, 500);

  p.parse_check(argc, argv);

  string dictPath =  p.get<string>("dictFile");
  string abstractPath =  p.get<string>("abstractFile");
  string centroidPath =  p.get<string>("centroidFile");
  bool verbose = p.exist("verbose");
  unsigned int vecSize = p.get<unsigned int>("vecSize");

  if(verbose) cout << "Started" << endl;

  if(verbose) cout << "Loading dict" << endl;
  Dict dictionary(dictPath);

  if(verbose) cout << "Making Centroids" << endl;

  fstream abstractFile(abstractPath, ios::in);
  fstream centroidFile(centroidPath, ios::out);

  Vec zero(vector<float>(vecSize, 0));

  string line;
  string key;
  string phrase;
  while(getline(abstractFile, line)){
    stringstream s;
    int count = 0;
    s << line;
    s >> key;
    Vec centroid(vecSize);
    while(s >> phrase){
      if(dictionary.contains(phrase)){
//        if(verbose){
//          cout << "Found " << phrase << " in dict." << endl
//               << centroid.toString() << " + " << dictionary.getVec(phrase).toString()
//               << " = " << (centroid+dictionary.getVec(phrase)).toString() << endl;
//        }
        centroid += dictionary.getVec(phrase);
        count++;
      }
    }
    if(count > 0){
//      if(verbose){
//        cout << "Average:"
//             << centroid.toString() << "/" << count << "="
//             << (centroid / count).toString() << endl;
//      }
      centroid /= count;
    }
    else
      centroid = zero;
    centroidFile << key << " " << centroid.toString() << endl;
  }
  abstractFile.close();
  centroidFile.close();

  if(verbose) cout << "Done, cleaning up" << endl;
}
