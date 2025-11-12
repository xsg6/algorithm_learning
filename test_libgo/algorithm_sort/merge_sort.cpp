#include<iostream>
#include<vector>
using namespace std;
// 归并排序函数
void merge(vector<int>& nums,int left,int mid,int right){
    vector<int> temp;
    int i=left,j=mid+1;
    while(i<=mid && j<=right){
        if(nums[i]<=nums[j]){
            temp.push_back(nums[i++]);
        }
        else{
            temp.push_back(nums[j++]);
        }
    }
    while(i<=mid){
        temp.push_back(nums[i++]);
    }
    while(j<=right){
        temp.push_back(nums[j++]);
    }
    for(int k=0;k<temp.size();k++){
        nums[left+k]=temp[k];
    }
}
void merge_sort(vector<int>& nums,int left,int right){
    if(left>=right)return;
    int mid=(left+ right)/2;
    merge_sort(nums,left,mid);
    merge_sort(nums,mid+1,right);
    // 合并两个有序子数组
    merge(nums,left,mid,right);
}