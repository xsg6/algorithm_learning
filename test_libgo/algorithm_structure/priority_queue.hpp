#include <iostream>
#include <vector>
class priority_queue{
private:
    std::vector<int> heap;
public:
    void siftUp(int child){};
    void siftDown(int root,int n){};
    int pop(){};
    int top(){};
    void push(int val){};
    explicit priority_queue(){};
    ~priority_queue(){};
};