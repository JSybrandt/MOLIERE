// CONVERT TEXT EDGES TO BINARY

#include<fstream>

using namespace std;

const string TEXT_PATH = "./final.edges";
const string BINARY_PATH = "./final.bin.edges";

union charBuff{
  unsigned int i;
  float f;
  char buff[4];
};

int main(){

  fstream fin(TEXT_PATH, ios::in);
  fstream fout(BINARY_PATH, ios::out|ios::binary);
  charBuff a, b, w;
  while(fin >> a.i >> b.i >> w.f){
    fout.write(a.buff, 4);
    fout.write(b.buff, 4);
    fout.write(w.buff, 4);
  }

}
