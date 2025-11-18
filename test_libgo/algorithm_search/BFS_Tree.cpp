#include <iostream>
#include <vector>
using namespace std;
/**
 * @brief Simple binary tree node storing an integer value and child pointers.
 *
 * Represents a node in a binary tree used for traversal and algorithmic operations
 * (e.g., BFS, DFS, tree construction). The node contains:
 *  - val: the integer value held by the node.
 *  - left: pointer to the left child (nullptr if none).
 *  - right: pointer to the right child (nullptr if none).
 *
 * The provided constructor initializes the node's value and sets both child pointers
 * to nullptr.
 *
 * Usage:
 *   TreeNode* node = new TreeNode(5); // val = 5, left = right = nullptr
 */
struct TreeNode
{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};
TreeNode *BFS(TreeNode *root, int target)
{
    if (root == nullptr)
    {
        return nullptr;
    }
    vector<TreeNode *> queue;
    queue.push_back(root);
    while (!queue.empty())
    {
        TreeNode *current = queue.front();
        queue.erase(queue.begin());
        if (current->val == target)
        {
            return current;
        }
        if (current->left != nullptr)
        {
            queue.push_back(current->left);
        }
        if (current->right != nullptr)
        {
            queue.push_back(current->right);
        }
    }
    return nullptr;
}