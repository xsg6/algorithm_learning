#include <iostream>
#include <vector>
using namespace std;
void shell_sort(vector<int> &nums)
{
    for (int gap = nums.size() / 2; gap > 0; gap /= 2)
    {
        for (int i = gap; i < nums.size(); i++)
        {
            int key = nums[i];
            int j = i - gap;
            while (j >= 0 && nums[j] > key)
            {
                nums[j + gap] = nums[j];
                j -= gap;
            }
            nums[j + gap] = key;
        }
    }
}