#include<iostream>
#include<vector>
using namespace std;
// 快速排序函数
void quickSort(vector<int>& nums, int left, int right) {
    if (left >= right){
        return ;
    }
    int pivot =nums[left]; //锚定基准值
    int i = left;
    int j = right;
    while(i<j){
        while(nums[j]>=pivot) j--;
        while(nums[i]<=pivot) i++;
        if(i<j){
            swap(nums[i],nums[j]);
        }
    }
    swap(nums[left],nums[j]); //将基准值放到正确位置
    quickSort(nums,left,j-1); //递归排序左半部分
    quickSort(nums,j+1,right); //递归排序右半部分
}