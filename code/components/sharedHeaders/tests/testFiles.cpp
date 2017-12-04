#include<iostream>
#include"../files.h"

using namespace std;

int main(){
  string dir = "/scratch2/jsybran/moliere/data/yearlySubsets/2010/fastText/parts";
  for(string fileName : getFilesInDir(dir)){
    cout << "Checking:" << join(dir, fileName) << endl;
    if(isFile(join(dir, fileName)))
      cout << fileName << endl;
  }
  dir = "/scratch2/jsybran/moliere/data/yearlySubsets/2010/fastText/parts/";
  for(string fileName : getFilesInDir(dir)){
    cout << "Checking:" << join(dir, fileName) << endl;
    if(isFile(join(dir, fileName)))
      cout << fileName << endl;
  }
  return 0;
}
