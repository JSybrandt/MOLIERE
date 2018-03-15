#pragma once
#include <dirent.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>

using std::vector;
using std::string;
using std::stringstream;
using std::runtime_error;

vector<string> getDirContents(const string& dirPath){
  DIR *dpdf;
  struct dirent *epdf;
  vector<string> res;

  dpdf = opendir(dirPath.c_str());
  if (dpdf != NULL){
     while((epdf = readdir(dpdf))){
        res.push_back(epdf->d_name);
     }
  } else {
    throw std::runtime_error("Failed to Open Directory");
  }
  closedir(dpdf);
  return res;
}

bool isFile(const string& path){
  struct stat s;
  if(stat(path.c_str(), &s) == 0){
    return s.st_mode & S_IFREG;
  }
  else{
    return false;
    //throw std::runtime_error("Unable to Open File");
  }
}

void assertIsFile(const string& path){
  if(! isFile(path))
    throw runtime_error("Failed to find " + path);
}

bool isDir(const string& path){
  struct stat s;
  if(stat(path.c_str(), &s) == 0){
    return s.st_mode & S_IFDIR;
  }
  else{
    return false;
    //throw std::runtime_error("Unable to Open Directory");
  }
}

void assertIsDir(const string& path){
  if(! isDir(path))
    throw runtime_error("Failed to find " + path);
}

string join(const string& a, const string& b){
  stringstream ss;
  ss << a;
  if(a[a.size()-1] != '/')
    ss << '/';
  ss << b;
  return ss.str();
}

vector<string> getFileOrFiles(const string& path){
  vector<string> res;
  if(isFile(path)){
    res.push_back(path);
  } else if(isDir(path)) {
    for(string fileName : getDirContents(path)){
      string filePath = join(path, fileName);
      if(isFile(filePath)){
        res.push_back(fileName);
      }
    }
  } else {
    throw std::runtime_error("Unable to load file or files.");
  }
  return res;
}
