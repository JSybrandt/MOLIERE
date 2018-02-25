#include "vec.h"

using namespace std;

Vec::Vec(vector<float> data){
	this->data = data;
}

Vec::Vec(int size){
	data = vector<float>(size, 0);
}

Vec::Vec(){
  data = vector<float>();
}

Vec & Vec::operator=(const Vec& other){
	this->data = other.data;
	return *this;
}

string Vec::toString() const{
  stringstream s;
	for (float f : data) {
		s << to_string(f) + " ";
	}
	return s.str();
}

Vec Vec::operator+(const Vec & other){
  if(this->size() != other.size()){
    cerr << "Attempted to operate on vectors of unequal lengths." << endl;
    exit(1);
  }

  vector<float> data = this->data;
  for (int i = 0; i < this->size(); i++) {
		 data[i] += other.data[i];
	}
	return Vec(data);
}

Vec Vec::operator/(float other){
  vector<float> data = this->data;
  for(float& f : data){
    f /= other;
  }
	return Vec(data);
}

void Vec::operator+=(const Vec & other){
  if(this->size() != other.size()){
    cerr << "Attempted to operate on vectors of unequal lengths." << endl;
    exit(1);
  }
	for (int i = 0; i < this->size(); ++i) {
		this->data[i] += other.data[i];
	}
}

void Vec::operator/=(float other){
  for(float& f : data){
    f /= other;
  }
}

double Vec::dot(const Vec& other) const{
  if(this->size() != other.size()){
    cerr << "Attempted to operate on vectors of unequal lengths." << endl;
    exit(1);
  }
    double r = 0;
    for(int i = 0; i < this->size(); ++i){
        r += this->data[i] * other.data[i];
    }
    return r;
}

double Vec::magnitude() const{
    double r = 0;
    for(float d : data)
        r += d * d;
    return sqrt(r);
}

double Vec::cosDist(const Vec& other) const{
  if(this->size() != other.size()){
    cerr << "Attempted to operate on vectors of unequal lengths." << endl;
    exit(1);
  }
    return 1 - (this->dot(other) / (this->magnitude() * other.magnitude()));
}

float Vec::get(int i) const{
    return this->data[i];
}

float Vec::size() const{
  return this->data.size();
}
