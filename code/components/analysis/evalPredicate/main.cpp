/*
 * The goal here is to calculate some statistics about each of the predicate TM's that we generate.
 * This means that we need to read a predicate, a TM file, and the synonyms of that word.
 *
 *
 * How hard is it to load this file... thats worth a check
 * It takes less than half a second to load the file, we can do that.
 */

#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<fstream>
#include<sstream>
#include<cmath>
#include"cmdln.h"

using namespace std;

typedef unordered_map<string, unordered_map<unsigned int, unsigned int>> TopicModel;
typedef unordered_map<string, unordered_set<string>> AtomDict;

bool verbose;

// this will make sure all ngrams are in the std format
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

unsigned int getTM(string path, TopicModel& result){
  static const string TOPIC_TOKEN = "TOPIC:";

  string token;
  unsigned int numTopics = 0;
  // Get number of topics
  fstream tmFile(path, ios::in);
  while(tmFile >> token){
    if(token == TOPIC_TOKEN){
      numTopics++;
    }
  }
  tmFile.close();

  tmFile = fstream(path, ios::in);
  unsigned int currTopic = 0;
  float occuranceRate;
  while(tmFile >> token){
    if(token == TOPIC_TOKEN){
      tmFile >> currTopic;
      tmFile >> occuranceRate;
    } else {
      tmFile >> occuranceRate;
      // NOTE: we have to load as float because that the file.
      // All the values are positive integers
      // token = processString(token);
      result[token][currTopic] = occuranceRate;
    }
  }
  tmFile.close();

  return numTopics;
}

void loadAtoms(string path, AtomDict& cuid2Atoms){
  fstream inFile(path);
  string line;
  while(getline(inFile, line)){
    stringstream s(line);
    string atom, cuid;
    s >> cuid;
    while(s >> atom){
      cuid2Atoms[cuid].insert(atom);
    }
  }
  inFile.close();
}

vector<float> getProbByTopic(const TopicModel& word2Topic,
                             const unordered_set<string>& words,
                             unsigned int numTopics){
  vector<float> probs(numTopics, 0);
  for(string word : words){
    auto it = word2Topic.find(word);
    if(it != word2Topic.end()){
      for(auto& pair : it->second){
        probs[pair.first] += pair.second;
      }
    } else if(::verbose){
      cerr << "Warning: failed to find " << word << " in topic model." << endl;
    }
  }
  float sum = 0;
  for(float i : probs){
    sum += i;
  }
  if(sum > 0){
    for(float& i : probs){
      i /= sum;
    }
  }
  return probs;
}

float getSimilarity(const vector<float>& probA, const vector<float>& probB){
  if(probA.size() == probB.size()){
    float msqerr = 0;
    for(unsigned int i = 0; i < probA.size(); i++){
      msqerr += pow(probA[i] - probB[i], 2);
    }
    msqerr /= probA.size();

    float similarity = 1 - msqerr;
    return similarity;
  }

  throw "GRABAGE INTO getSimilarity";
  return -1;
}

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("topicModel", 'm', "Topic model from VIEW_FILES", true);
  p.add<string>("MRCONSO", 'M', "Post-processed MRSONSO file.", true);
  p.add<string>("sourceCUID", 's', "Source CUID.", true);
  p.add<string>("targetCUID", 't', "Target CUID.", true);
  p.add<string>("verb", 'b', "verb.", true);
  p.add("verbose", 'v', "Output debug info.");

  p.parse_check(argc, argv);
  string topicModelPath = p.get<string>("topicModel");
  string mrconsoPath = p.get<string>("MRCONSO");
  string sourceCUID = p.get<string>("sourceCUID");
  string targetCUID = p.get<string>("targetCUID");
  string verb = p.get<string>("verb");
  ::verbose = p.exist("verbose");

  TopicModel word2Topics;
  unsigned int numTopics = getTM(topicModelPath, word2Topics);

  if(::verbose){
    cout << "Identified " << numTopics << " topics." << endl;
    cout << "Loaded " << word2Topics.size() << " words." << endl;
  }

  unordered_set<string> sourceAtoms, targetAtoms;

  fstream inFile(mrconsoPath);
  string line, atom, cuid;
  while(getline(inFile, line)){
    stringstream s(line);
    s >> cuid;
    if(cuid == sourceCUID){
      while(s >> atom){
        sourceAtoms.insert(atom);
      }
    }
    if(cuid == targetCUID){
      while(s >> atom){
        targetAtoms.insert(atom);
      }
    }
  }
  inFile.close();

  if(sourceAtoms.size() == 0){
    cerr << "Failed to find atoms for " << sourceCUID << endl;
    exit(1);
  }
  if(targetAtoms.size() == 0){
    cerr << "Failed to find atoms for " << targetCUID << endl;
    exit(1);
  }

  if(::verbose){
    cout << "Found " << sourceAtoms.size() << " atoms for " << sourceCUID << endl;
    cout << "Found " << targetAtoms.size() << " atoms for " << targetCUID << endl;
  }

  vector<float> sourceProb = getProbByTopic(word2Topics, sourceAtoms, numTopics);
  vector<float> targetProb = getProbByTopic(word2Topics, targetAtoms, numTopics);

  if(::verbose){
    cout << "Prob for " << sourceCUID << endl;
    for(unsigned int i = 0; i < numTopics; i++){
      if(sourceProb[i] > 0)
        cout << "\t" << i << "\t" << sourceProb[i] << endl;
    }
    cout << "Prob for " << targetCUID << endl;
    for(unsigned int i = 0; i < numTopics; i++){
      if(targetProb[i] > 0)
        cout << "\t" << i << "\t" << targetProb[i] << endl;
    }
  }

  float sim = getSimilarity(sourceProb, targetProb);
  if(::verbose){
    cout << "P(t|x) sim P(t|y) " << sim << endl;
  }

  return 0;
}
