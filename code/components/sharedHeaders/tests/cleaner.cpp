#include<iostream>

#include <clocale>
#include <cwctype>

#include "../cleanText.h"

using namespace std;

int main(){
  setlocale(LC_ALL, "en_US.UTF-8");
  string tmp;
  while(getline(cin, tmp)){
    cout << cleanText(tmp) << endl;
  }
  return 0;
}

