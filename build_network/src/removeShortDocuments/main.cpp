#include<iostream>
#include<unordered_map>
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

#include"cmdln.h"
#include"parallelFileOp.h"
#include"bow.h"

bool verbose = false;
#define vout if(::verbose) cout

using namespace std;


/*
 * GOAL: Remove short documents that don't contain at least two "rare" words
 * Rare determined by DF
 */
int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("input", 'i', "each line of this file is a record", true);
  p.add<string>("output", 'o', "output file", true);
  p.add<size_t>("min-length", 'm', "documents with wordcount above this will be automatically included", false, 20);
  p.add<float>("common-fraction", 'c', "fraction [0-1] determining the ratio of common to rare words. Higher = more common words", false, 0.2f);
  p.add("verbose", 'v', "outputs debug information");
  p.add("skip-second", 's', "skips the second word of each record (in addition to the first)");
  p.add<size_t>("min-rare-count", 'r', "number of rare words needed for a short doc to count.", false, 2);

  p.parse_check(argc, argv);

  string inputPath =  p.get<string>("input");
  string outPath =  p.get<string>("output");
  size_t minLength =  p.get<size_t>("min-length");
  float commonFraction =  p.get<float>("common-fraction");
  verbose = p.exist("verbose");
  bool skipSecond = p.exist("skip-second");
  size_t minRareCount = p.get<size_t>("min-rare-count");

  vout << "Processing" << endl;
  auto line2occurBow = [&skipSecond](const string& line) -> Bow{
    return Bow(line, (skipSecond ? 2 : 1)).toOccurances();
  };

  list<Bow> bagList;
  fastProcFile<Bow>(inputPath, bagList, line2occurBow);

  vout << "List 2 vec " << endl;
  vector<Bow> bags;
  bags.reserve(bagList.size());
  for(auto& p : bagList)
    bags.emplace_back(move(p));

  vout << "Tabulating in parallel" << endl;
  Bow docCounts;
  #pragma omp parallel
  {
    Bow localCounts;
    #pragma omp for schedule(dynamic) nowait
    for(size_t i = 0; i < bags.size(); ++i){
      localCounts.add(bags[i]);
    }
    #pragma omp critical(counts)
    docCounts.add(localCounts);
  }

  vout << "Determinging cutoff " << endl;
  vector<size_t> counts;
  counts.reserve(docCounts.getCounts().size());
  for(const auto& p : docCounts.getCounts()){
    counts.emplace_back(p.second);
  }
  // sort in reverse order
  sort(counts.begin(), counts.end(), std::greater<size_t>());

  // for a word to be "rare" it needs to occur less than the cutoff
  size_t commonCutoff = counts[int(counts.size()*commonFraction)];

  vout << "Determinied a min cutoff of " << commonCutoff << endl
       << "A word must occur in less than "
       << (float(commonCutoff) / float(bags.size())) * 100 << "% of medline to be rare."  << endl;

  vout << "keeping only long or important docs." << endl;
  auto filter = [&skipSecond, &minRareCount, &minLength, &commonCutoff, &docCounts]
                (const string& line) -> bool
  {
    Bow words(line, (skipSecond ? 2 : 1));
    size_t rare_count = 0;
    size_t word_count = 0;
    for(const auto& p : words.getCounts()){
      if(docCounts.getCount(p.first) < commonCutoff)
        rare_count += 1;
      word_count += p.second;
      if(word_count > minLength) return true;
      if(rare_count > minRareCount) return true;
    }
    return false;
  };
  auto identity = [](const string& line) -> string {return line;};

  list<string> filteredText;
  fastProcFile<string>(inputPath, filteredText, identity, filter);

  vout << "writing" << endl;
  fstream outFile(outPath, ios::out);
  for(const string& str : filteredText){
    outFile << str << endl;
  }
  outFile.close();

  return 0;
}
