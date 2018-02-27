#include<iostream>
#include<sstream>
#include<fstream>

#include <clocale>
#include <cwctype>

#include"cmdln.h"
#include"pugixml.hpp"
#include"omp.h"
#include"cleanText.h"

bool verbose = false;

using namespace::std;

#define vout if(::verbose) cout
#define vwout if(::verbose) wcout

const string RETRACTED = "Retracted";
const string COMMENT = "Comment";
const string RETRACTED_PUB = "Retracted Publication";

int main(int argc, char ** argv){
  cmdline::parser p;

  p.add<string>("file", 'f', "MEDLINE file to parse", true);
  p.add<string>("output", 'o', "result file", true);
  p.add("include-keywords", 'k',
        "if true, keywords provided by authors are appended to abstracts.");
  p.add("only-keywords", 'O', "outputs pmid, keyword list from author-defined tags.");
  p.add("no-metadata", 'N', "silences pmid and year.");
  p.add("verbose", 'v', "Output debug info.");

  p.parse_check(argc, argv);
  string inPath = p.get<string>("file");
  string outPath = p.get<string>("output");
  ::verbose = p.exist("verbose");
  bool includeKeywords = p.exist("include-keywords");
  bool onlyKeywords = p.exist("only-keywords");
  bool noMeta = p.exist("no-metadata");
  includeKeywords = onlyKeywords || includeKeywords;


  vout << "Setting Locale" << endl;
  setlocale(LC_ALL, "en_US.UTF-8");

  vout << "Creating Document" << endl;
  pugi::xml_document doc;

  vout << "Loading " << inPath << endl;
  pugi::xml_parse_result result = doc.load_file(inPath.c_str());

  vout << "Load Result: " << result.description() << endl;
  if(! result){
    cerr << "FAILED TO PARSE " << inPath;
    exit(1);
  }

  vout << "Opening " << outPath << endl;
  fstream outFile(outPath, ios::out);

  unsigned int count = 0, brokenRecords = 0, missingYear = 0, retracted = 0, comment = 0;
  for(pugi::xml_node pubmedArticleNode : doc.child("PubmedArticleSet").children("PubmedArticle")){
//    try{
      pugi::xml_node citationNode = pubmedArticleNode.child("MedlineCitation");
      pugi::xml_node pmidNode = citationNode.child("PMID");
      pugi::xml_node articleNode = citationNode.child("Article");
      pugi::xml_node titleNode = articleNode.child("ArticleTitle");
      pugi::xml_node abstractNode = articleNode.child("Abstract");
      pugi::xml_node pubTypesNode = articleNode.child("PublicationTypeList");
      pugi::xml_node keywordListNode = citationNode.child("KeywordList");

      // if retracted type
      if(pubTypesNode){
        bool skip = false;
        for(pugi::xml_node pubTypeNode : pubTypesNode.children("PublicationType")){
          if(pubTypeNode.text().get() == RETRACTED_PUB){
            skip = true;
            retracted += 1;
            break;
          }
          if(pubTypeNode.text().get() == COMMENT){
            skip = true;
            comment += 1;
            break;
          }
        }
        if(skip){
          continue;
        }
      }

      // if retracted title
      string title = titleNode.text().get();
      if(title.substr(0, RETRACTED.size()) == RETRACTED){
        retracted += 1;
        continue;
      }

      // if doesn't contain right fields
      if(! citationNode || ! pmidNode || ! articleNode || ! titleNode){
        brokenRecords += 1;
        continue;
      }

      count += 1;

      int year = numeric_limits<int>::max();
      for(pugi::xml_node dateNode : pubmedArticleNode.child("PubmedData")
                                                     .child("History")
                                                     .children("PubMedPubDate")){
        year = min(year, stoi(dateNode.child("Year").text().get()));
      }

      if(year == numeric_limits<int>::max()){
        missingYear += 1;
      }

      string pmid = pmidNode.text().get();
      string version = pmidNode.attribute("Version").value();

      if(!noMeta){
        outFile << "PMID" << pmid << "_" << version << " "
                << year << " . ";
      }

      stringstream text;
      if(!onlyKeywords){

        text << title << " . ";
        if(abstractNode){
          stringstream abTextSS;
          for(pugi::xml_node textNode : abstractNode.children("AbstractText")){
            text << textNode.text().get() << " . ";
          }
        }
      }

      if(keywordListNode && includeKeywords){
        for(pugi::xml_node keyNode : keywordListNode.children("Keyword")){
          string k = keyNode.text().get();
          transform(k.begin(), k.end(), k.begin(),
              [](char c) -> char {return (c==' ' ? '_' : c);});
           text << k << " . ";
        }
      }

      outFile << cleanText(text.str()) << endl;

    //} catch(...){
      //vout << "Failed to parse something from " << inPath << endl;
    //}
  }

  vout << "Parse Summary:" << endl
       << "\t      Correct:" << count << endl
       << "\t Missing Year:" << missingYear << endl
       << "\tMissing Entry:" << brokenRecords << endl
       << "\t    Retracted:" << retracted << endl
       << "\t      Comment:" << comment << endl;


  vout << "Closing " << outPath << endl;
  outFile.close();
}
