#include<iostream>
#include<queue>
#include<vector>
#include<stack>
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
public:
    Graph(int n, bool directed = false) : nodeCount(n), isDirected(directed) {
        adjMatrix.resize(n, vector<int>(n, INF));
        for (int i = 0; i < n; ++i) {
            adjMatrix[i][i] = 0;
        }   
    }
    void addEdge(int u, int v, int weight = 1) {
        adjMatrix[u][v] = weight;
        if (!isDirected) {
            adjMatrix[v][u] = weight;
        }
    }
    void addNode(){
        nodeCount++;
        for(auto &row:adjMatrix){
            row.push_back(INF);
        }
        adjMatrix.push_back(vector<int>(nodeCount,INF));
        adjMatrix[nodeCount-1][nodeCount-1]=0;
    }
    void removeEdge(int u, int v) {
        adjMatrix[u][v] = INF;
        if (!isDirected) {
            adjMatrix[v][u] = INF;
        }
    }
    vector<int>BFS(const int& start){
        vector<int>visited(nodeCount,0);
        vector<int>order;
        queue<int>q;
        q.push(start);
        visited[start]=1;
        while(!q.empty()){
            int curr=q.front();
            q.pop();
            order.push_back(curr);
            for(int i=0;i<nodeCount;++i){
                if(adjMatrix[curr][i]!=INF && !visited[i]){
                    visited[i]=1;
                    q.push(i);
                }
            }
        }
        return order;
    }
    vector<int>DFS(const int& start){
        vector<int>visited(nodeCount,0);
        vector<int>order;
        stack<int>s;
        s.push(start);
        while(!s.empty()){
            int curr=s.top();
            s.pop();
            if(!visited[curr]){
                visited[curr]=1;
                order.push_back(curr);
                for(int i=nodeCount-1;i>=0;--i){
                    if(adjMatrix[curr][i]!=INF && !visited[i]){
                        s.push(i);
                    }
                }
            }
        }
        return order;
    }   
};