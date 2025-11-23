#include<iostream>
#include<queue>
#include<vector>
using namespace std;
template <typename T,typename weightType=int>
struct Edge
{
    T traget;
    weightType weight;
    Edge(T t,weightType w=1):traget(t),weight(w){}
};

class Graph {
private:
    int nodeCount;
    vector<vector<int>>adjMatrix;
    bool isDirected;
    const int INF = 1e9;

};