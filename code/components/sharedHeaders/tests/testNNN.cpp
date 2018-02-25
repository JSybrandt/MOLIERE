#include<iostream>
#include"../exactNNN.h"

using namespace std;

int main(){
  vector<vector<float>> points = {
    {1, 1},
    {1, -1},
    {-1, -1},
    {-1, 1},
    {1.3, 1.2},
    {1.3, -1.2},
    {-1.3, -1.2},
    {-1.3, 1.2},
    {0, 1.2},
    {0, -1.2},
    {1.2, 0},
    {-1.2, 0}
  };

  //cout << "Vectors: " << endl;
  //unsigned int count = 0;
  //for(auto& vec : points){
    //cout << count << " : ";
    //for(float f : vec){
      //cout << f << " ";
    //}
    //cout << endl;
    //++count;
  //}

  try{
    cout << "L2" << endl;
    Graph g = createNNN(points, 2, true);
    cout << g << endl;
    Graph::Path path = g.getShortestPath(0, 5);
    cout << path << endl;
  } catch(runtime_error e){
    cout << e.what() << endl;
  }
  try{
    cout << "COS" << endl;
    Graph g = createNNN(points, 2, false);
    cout << g << endl;
    Graph::Path path = g.getShortestPath(0, 5);
    cout << path << endl;
  } catch(runtime_error e){
    cout << e.what() << endl;
  }
}
