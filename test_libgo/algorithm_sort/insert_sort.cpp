#include <iostream>
#include <vector>
using namespace std;
void insert_sort(vector<int> &nums)
{
    if (nums.size() <= 1)
        return;
    int key = 0;
    int front = 0;
    for (int i = 1; i < nums.size(); i++)
    {
        key = nums[i];
        front = i - 1;
        while (front >= 0 && nums[front] > key)
        {
            {
                nums[front + 1] = nums[front];
                front--;
            }
        }
        nums[front + 1] = key;
    }
}