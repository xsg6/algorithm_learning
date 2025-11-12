#include<iostream>
#include<vector>
using namespace std;
// 二分查找函数，返回目标值在数组中的索引，找不到返回 -1
int binarySearcsh(vector<int>& nums,int target){
    int left=0;
    int right=nums.size()-1;
    int mid=0;
    while(left<=right){
        mid=(left+right)/2;
        if(nums[mid]==target){
            return mid;
        }
        else if(nums[mid]<target){
            left=mid+1;
        }
        else{
            right=mid-1;
        }
    }
    return -1;
}