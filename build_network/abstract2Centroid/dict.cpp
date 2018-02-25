#include"dict.h"

using namespace std;
Dict::Dict(string file) {
	ifstream dataFile(file);
	string line;
	bool firstLine = true;
	while (getline(dataFile,line)) {
	  stringstream s;
		s << line;
		if (firstLine) {
      firstLine = false;
      s >> this->numRecords;
      s >> this->vecSize;
    } else if (line.length() > 0) {
			string word;
			s >> word;
			vector<float> vectorData;
			float d;
			while (s >> d) {
				vectorData.push_back(d);
			}
      try{
			  data[word] = Vec(vectorData);
      }catch(...){
        cerr << "Failed to load [" << word << "] into dict" << endl;
      }
		}
	}
	dataFile.close();
}

const Vec Dict::getVec(string word) {
	return data[word];
}

const bool Dict::contains(string word){
  return data.find(word) != data.end();
}
