#pragma once

#include <vector>
#include<exception>
#include<string>
#include<iostream>
#include<sstream>

#include<cmath>

using std::vector;
using std::string;
using std::stringstream;




class Vec {
private:
	vector<float> data;

public:
	Vec(vector<float> data);
	Vec(int size);
  Vec();
	Vec(const Vec&) = default;
	Vec& operator=(const Vec&);
	std::string toString() const;
	Vec operator+(const Vec& other);
	Vec operator/(float other);
	void operator+=(const Vec& other);
	void operator/=(float other);

  double dot(const Vec& other) const;
  double magnitude() const;
  double cosDist(const Vec& other) const;
  float get(int i) const;
  float size() const;
};
