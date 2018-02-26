#pragma once

#include<string>
#include<cstring>
#include<cwctype>
#include<iostream>
#include<unordered_map>
#include<sstream>
#include<stack>
#include<vector>
#include<list>

#include<iconv.h>

#include"porter2_stemmer.h"
// MUST BE LINKED WITH OBJ

using std::string;
using std::wstring;
using std::unordered_map;
using std::stringstream;
using std::stack;
using std::vector;
using std::list;

const unordered_map<string, string> symbol2text = {
  {u8"\u0391", "alpha"},
  {u8"\u0392", "beta"},
  {u8"\u0393", "gamma"},
  {u8"\u0394", "delta"},
  {u8"\u0395", "epsilon"},
  {u8"\u0396", "zeta"},
  {u8"\u0397", "eta"},
  {u8"\u0398", "theta"},
  {u8"\u0399", "iota"},
  {u8"\u039A", "kappa"},
  {u8"\u039B", "lambda"},
  {u8"\u039C", "mu"},
  {u8"\u039D", "nu"},
  {u8"\u039E", "xi"},
  {u8"\u039F", "omicron"},
  {u8"\u03A0", "pi"},
  {u8"\u03A1", "rho"},
  {u8"\u03A3", "sigma"},
  {u8"\u03A4", "tau"},
  {u8"\u03A5", "upsilon"},
  {u8"\u03A6", "phi"},
  {u8"\u03A7", "chi"},
  {u8"\u03A8", "psi"},
  {u8"\u03A9", "omega"},
  {u8"\u03B1", "alpha"},
  {u8"\u03B2", "beta"},
  {u8"\u03B3", "gamma"},
  {u8"\u03B4", "delta"},
  {u8"\u03B5", "epsilon"},
  {u8"\u03B6", "zeta"},
  {u8"\u03B7", "eta"},
  {u8"\u03B8", "theta"},
  {u8"\u03B9", "iota"},
  {u8"\u03BA", "kappa"},
  {u8"\u03BB", "lambda"},
  {u8"\u03BC", "mu"},
  {u8"\u03BD", "nu"},
  {u8"\u03BE", "xi"},
  {u8"\u03BF", "omicron"},
  {u8"\u03C0", "pi"},
  {u8"\u03C1", "rho"},
  {u8"\u03C3", "sigma"},
  {u8"\u03C4", "tau"},
  {u8"\u03C5", "upsilon"},
  {u8"\u03C6", "phi"},
  {u8"\u03C7", "chi"},
  {u8"\u03C8", "psi"},
  {u8"\u03C9", "omega"},
  {u8"·", "."},
  {u8"⁻", "^-"},
  {u8"⁰", "^0"},
  {u8"¹", "^1"},
  {u8"²", "^2"},
  {u8"³", "^3"},
  {u8"⁴", "^4"},
  {u8"⁵", "^5"},
  {u8"⁶", "^6"},
  {u8"⁷", "^7"},
  {u8"⁸", "^8"},
  {u8"⁹", "^9"},
  {u8"⁺", "^+"},
  {u8"⁻", "^-"},
  {u8"⁼", "^="},
  {u8"⁽", "^("},
  {u8"⁾", "^)"},
  {u8"ⁿ", "^n"},
  {u8"ⁱ", "^i"},
  {u8"₀", "_0"},
  {u8"₁", "_1"},
  {u8"₂", "_2"},
  {u8"₃", "_3"},
  {u8"₄", "_4"},
  {u8"₅", "_5"},
  {u8"₆", "_6"},
  {u8"₇", "_7"},
  {u8"₈", "_8"},
  {u8"₉", "_9"},
  {u8"₊", "_+"},
  {u8"₋", "_-"},
  {u8"₌", "_="},
  {u8"₍", "_("},
  {u8"₎", "_)"},
  {u8"ₐ", "_a"},
  {u8"ₑ", "_e"},
  {u8"ₕ", "_h"},
  {u8"ᵢ", "_i"},
  {u8"ⱼ", "_j"},
  {u8"ₖ", "_k"},
  {u8"ₗ", "_l"},
  {u8"ₘ", "_m"},
  {u8"ₙ", "_n"},
  {u8"ₒ", "_o"},
  {u8"ₚ", "_p"},
  {u8"ᵣ", "_r"},
  {u8"ₛ", "_s"},
  {u8"ₜ", "_t"},
  {u8"ᵤ", "_u"},
  {u8"ᵥ", "_v"},
  {u8"ₓ", "_x"},
  {u8"ₔ", "_@"},
  {u8"∞", "inf"},
  {u8"±", "+/-"},
  {u8"°", "degree"},
  {u8"∘", "smallcircle"},
  {u8"о", "bigcircle"},
  {u8"⩽", "<="},
  {u8"⩾", ">="},
  {u8"≡", "identical"},
  {u8"≠", "neq"},
  {u8"˂", "^<"},
  {u8"׳", ""},
  {u8"、", ""},
  {u8"ᅟ", ""}, //NOT THE SAME SPACE
  {u8"£", "pound"},
  {u8"¥", "yen"},
  {u8"¢", "cent"},
  {u8"∆", "delta"},
  {u8"△", "bigtri"},
  {u8"▵", "smalltri"},
  {u8"⋯", "..."},
  {u8"ƒ", "f"},
  {u8"ϕ", "phi"},
  {u8"Е", "E"},
  {u8"С", "C"},
  {u8"‰", "\%o"},
  {u8"§", "sec"},
  {u8"∑", "sum"},
  {u8"≈", "approx"},
  {u8"÷", "div"},
  {u8"Ø", "null"},
  {u8"ø", "o"},
  {u8"ĸ", "kra"},
  {u8"″", ""},
  {u8"\u202e", ""},
  {u8"\u202d", ""},
  {u8"\u202c", ""},
  {u8"♂", "male"},
  {u8"♀", "female"},
  {u8"≲", "<~"},
  {u8"≳", ">~"},
  {u8"≧", ">="},
  {u8"≦", "<="},
  {u8"≃", "approx"},
  {u8"≅", "approx"},
  {u8"↑", "up"},
  {u8"↓", "down"},
  {u8"●", "bigdot"},
  {u8"∙", "bullet"},
  {u8"⋅", "dot"},
  {u8"˙", "^dot"},
  {u8"∧", "^"},
  {u8"═", "="},
  {u8"‖", "||"},
  {u8"Ƞ", "n"},
  {u8"ƞ", "n"},
  {u8"Л", "el"},
  {u8"Ф", "ef"},
  {u8"р", "er"},
  {u8"Н", "en"},
  {u8"∷", "::"},
  {u8"⟩", ">"},
  {u8"〉", ">"},
  {u8"⟨", "<"},
  {u8"〈", ">"},
  {u8"΄", ""},
  {u8"∠", "angle"},
  {u8"ϵ", "epsilon"},
  {u8"ɛ", "epsilon"},
  {u8"√", "sqrt"},
  {u8"℃", "degreec"},
  {u8"⇄", "<=>"},
  {u8"⇆", "<=>"},
  {u8"", "<=>"},
  {u8"？", ""},
  {u8"‡", ""},
  {u8"⟶", "->"},
  {u8"ı", "_1"},
  {u8"Ӕ", "ae"},
  {u8"¿", ""},
  {u8"⊂", "subset"},

};


string toAscii(const string& uni){
  static iconv_t conv = iconv_open("ASCII//TRANSLIT", "UTF-8");
  size_t inSize = uni.size() + 1;
  char* inbuff = new char[inSize];
  strcpy(inbuff, uni.c_str());
  size_t outSize = inSize*2;
  char* outbuff = new char[outSize];
  char* outStart = outbuff;

  if(iconv(conv, &inbuff, &inSize, &outbuff, &outSize) < 0){
    std::cout << "FAILED UTF-8 -> ASCII" << std::endl;
  }
  return string(outStart);
}

string getVarWidthChar(const string& str, size_t i = 0){
    int charLen = 1;
    if((str[i] * 0xf8) == 0xf0) charLen = 4;
    else if((str[i] & 0xf0) == 0xe0) charLen = 3;
    else if((str[i] & 0xe0) == 0xc0) charLen = 2;
    if((i + charLen) > str.length()) charLen = 1;
    return str.substr(i, charLen);
}

// cleans dirty in place
string cleanText(const string& dirty){

  // we are going to add ss to workingText when we see parentheticals
  stack<stringstream> workingText;
  // when parentheticals are over, we will move from working to finished
  list<string> finishedText;

  workingText.emplace();

  bool skipNextParen = false;
  // iterate through mixed width characters
  for(size_t i = 0; i < dirty.length(); ){
    if(workingText.size() == 0){
      workingText.emplace();
      std::cerr << "Something weird happened" << std::endl;
    }
    stringstream& ss = workingText.top();

    string varWidthChar = getVarWidthChar(dirty, i);
    size_t charLen = varWidthChar.length();
    i += charLen;

    // if is multi-byte character
    if(charLen > 1){
      if(symbol2text.find(varWidthChar) != symbol2text.end()){
        const string &rep = symbol2text.at(varWidthChar);
        if(rep.size() > 1)
          ss << "{" << symbol2text.at(varWidthChar) << "}";
        else if (rep.size() == 1)
          ss << symbol2text.at(varWidthChar);
      } else {
        string ascii = toAscii(varWidthChar);
        //if(ascii == "?"){
          //std::cout << "Replacing " << varWidthChar << " with ?" << std::endl;
        //}
        ss << ascii;
      }
    } else { // is regular character
      char c = varWidthChar[0];
      // one symbol lookahead
      string nextVarWidthChar = getVarWidthChar(dirty, i);
      char nextChar;

      if(i >= dirty.length()) nextChar = '\n';
      else if(nextVarWidthChar.size() == 1) nextChar = nextVarWidthChar[0];
      else nextChar = '?';

      switch(c){
        // start parenthetical
        case '[': case '{': case '(':
          if(!skipNextParen)
            workingText.emplace();
          skipNextParen = false;
          break;
        // end parenthetical
        case ']': case '}': case ')':
          if(workingText.size() > 1){ // someone might list a) b) c)
            // if this parens might mean something
            if(!isspace(nextChar) &&
               !iscntrl(nextChar) &&
               !isblank(nextChar) &&
               nextChar != '.')
            {
              string tmp = ss.str();
              workingText.pop();
              workingText.top() << tmp; // reinsert text
            } else {
              finishedText.emplace_front(ss.str());
              workingText.pop();
            }
          }
          break;
        case '-':
          ss << '_'; break; // NOTE: we want hiv_ and {alpha}_
        case '?': case '!': case ';': case ':': case '.':
          if(isspace(nextChar)){  // punct is removed or tokenized
            ss << " . ";
          }
          break;
        case '\"': case '\'': case ',':
          break; //skip
        default:
          if((isspace(c) || iscntrl(c) || isblank(c))){
            // if this is a space and the next char does not start a parenthetical phrase
            // and is not another space
            if (nextChar != '(' && nextChar != '[' && nextChar != '{' &&
                !isspace(nextChar) && !iscntrl(nextChar) && !(isblank(nextChar)))
              ss << ' ';
          } else {
            ss << char(tolower(c));
            if(nextChar == '[' || nextChar == '{' || nextChar == '('){
              skipNextParen = true; // parens in a function may be important
            }
          }
          break;
      }
    }

  }
  // there are people who forget to put a )
  while(workingText.size() > 0){
    finishedText.emplace_front(workingText.top().str());
    workingText.pop();
  }
  stringstream ss;
  for(string& str : finishedText){
    if(str.size() > 0){
      bool containsInfo = false;
      for(char c : str){
        if(isalpha(c) || isdigit(c)){
          containsInfo = true;
          break;
        }
      }
      if(containsInfo){
        const auto begin = str.find_first_not_of(" _");
        const auto end = str.find_last_not_of(" _.");
        const auto size = end - begin + 1;
        if(ss.str().size() > 0) ss << " ";
        ss << str.substr(begin, size) << " . ";
      }
    }
  }

  string tmp;
  stringstream res;
  bool lastPeriod = false;
  while(ss >> tmp){
    if(tmp == "."){
      if(!lastPeriod)
        res << ". ";
      lastPeriod = true;
    }
    else{

      bool containsInfo = false;
      for(char c : tmp){
        if(isalpha(c) || isdigit(c)){
          containsInfo = true;
          break;
        }
      }
      if(containsInfo){ // only include tokens that have at least one letter or number
        lastPeriod = false;
        //this removes stray - or symbols like {+/-}
        Porter2Stemmer::stem(tmp);
        res << tmp << " ";
      }
    }
  }
  return res.str();
}


