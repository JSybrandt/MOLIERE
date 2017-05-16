/*
 * dijkstra2Datafiles
 *
 * We are going to take in a dijkstra file from the SP query step
 * and we are going to create bag files for each path...
 *
 * At this point, I think I need a database
 *
 */

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
#include <sqlite3.h>

#include"cmdln.h"
#include"bagOfWords.h"

using namespace std;

bool verbose = false;

/*
 * This class loads abstracts from the abstracts db
 * This class will cache bag results
 */

class abstractBagLoader{
  sqlite3 *db;
  char *dbErr = 0;
  unordered_map<string,bagOfWords> pmid2bag;
public:
  abstractBagLoader(string dbPath){
    sqlite3_open(dbPath.c_str(), &db);
  }
  ~abstractBagLoader(){
    sqlite3_close(db);
  }
  bagOfWords getBag(string pmid){
    if(pmid2bag.find(pmid) == pmid2bag.end()){
      string abText = loadAbstractText(pmid);
      if(verbose) cout << "Loading: " << pmid << "," << abText << endl;
      pmid2bag[pmid] = bagOfWords(abText);
    }
    return pmid2bag[pmid];
  }

private:
  string loadAbstractText(string pmid){
    sqlite3_stmt *statement;
    const char * query = ("select content from abstracts where pmid="+pmid+";").c_str();
    if(sqlite3_prepare(db, query, -1, &statement, 0) == SQLITE_OK){
      if(sqlite3_step(statement) == SQLITE_ROW){
        string abText = (char*) sqlite3_column_text(statement, 0);
        if(verbose) cout << "Recovered from db: " << abText << endl;
        return abText;
      } else {
        cerr << "Statement did not return row" << endl;
      }
    } else {
      cerr << "Failed to load " << pmid << endl;
    }
    return "";
  }
};

bool isPMID(string text){
  return all_of(text.begin(), text.end(), ::isdigit);
}

int main (int argc, char** argv){

  cmdline::parser p;

  p.add<string>("dijkstraFile", 'd', "dijkstra file from query", true);
  p.add<string>("graphLabelFile", 'g', "label file from graph", true);
  p.add<string>("abstractDB", 'a', "file path to sqlite abstract.db", true);
  p.add<string>("outputFileDir", 'o', "output dir for data files", true);
  p.add("verbose", 'v', "outputs debug information");
  p.add<unsigned int>("neighSize", 'n', "number of nearby abstracts to include", false, 1000);

  p.parse_check(argc, argv);

  string dijkPath =  p.get<string>("dijkstraFile");
  string dbPath =  p.get<string>("abstractDB");
  string labelPath =  p.get<string>("graphLabelFile");
  string outDirPath = p.get<string>("outputFileDir");
  verbose = p.exist("verbose");

  abstractBagLoader loader(dbPath);
  string pmid;
  while(cin >> pmid){
    cout << loader.getBag(pmid).toString() << endl;
  }

  return 0;
}
