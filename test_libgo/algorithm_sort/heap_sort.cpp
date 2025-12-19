#include<iostream>
#include<vector>
using namespace std;
class heapSort{
    private:
        vector<int> arr;
    public:
        heapSort(const vector<int>& input):arr(input){}
        void heapify(int n, int i){
            int largest = i;
            int left = 2*i + 1;
            int right = 2*i + 2;
            if(left < n && arr[left] > arr[largest])
                largest = left;
            if(right < n && arr[right] > arr[largest])
                largest = right;
            if(largest != i){
                swap(arr[i], arr[largest]);
                heapify(n, largest);
            }
        }
        void sort(){
            int n = arr.size();
            for(int i = n / 2 - 1; i >= 0; i--)
                heapify(n, i);
            for(int i = n - 1; i >= 0; i--){
                swap(arr[0], arr[i]);
                heapify(i, 0);
            }
        }
};