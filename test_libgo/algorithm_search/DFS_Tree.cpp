#include <iostream>
#include <vector>
using namespace std;
struct TreeNode
{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};
/**
 * @brief Recursively performs a depth-first (preorder) search in a binary tree to find a node with a specific value.
 *
 * 功能：在二叉树中以递归深度优先（先序）方式查找第一个值等于 target 的节点。
 *
 * Behaviour / 行为:
 *  - 检查当前节点（root），若匹配则立即返回该节点指针。
 *  - 否则先在左子树中递归查找；若在左子树找到则返回左子树的结果。
 *  - 若左子树未找到，再在右子树中递归查找并返回结果（若未找到则返回 nullptr）。
 *
 * @param root Pointer to the root of the (sub)tree to search. 指向要查找的树或子树的根节点指针；可为 nullptr。
 * @param target The value to search for. 要查找的目标值。
 *
 * @return Pointer to the TreeNode whose val equals target, or nullptr if no such node exists.
 *         返回值：指向值等于 target 的节点指针；若未找到或输入 root 为 nullptr，则返回 nullptr。
 *
 * @note
 *  - This implementation performs a preorder-style DFS (root → left → right) and returns the first match encountered.
 *    实现为先序 DFS（根→左→右），并返回遇到的第一个匹配节点。
 *  - Time complexity: O(n) in the worst case, where n is the number of nodes visited.
 *    时间复杂度：最坏 O(n)。
 *  - Space complexity: O(h) recursion stack, where h is the height of the tree. May overflow the call stack for very deep trees.
 *    空间复杂度：递归栈 O(h)，h 为树高；非常深的树可能导致栈溢出。
 *  - Comparison uses operator== on node values; ensure the type of val supports equality comparison.
 *    使用 == 进行比较，确保节点值类型支持相等比较。
 *
 * @warning Not thread-safe if the tree is concurrently modified. 若树在并发环境中被修改，则非线程安全。
 */
TreeNode *DFS(TreeNode *root, int target)
{
    if (root == nullptr)
    {
        return nullptr;
    }
    if (root->val == target)
    {
        return root;
    }
    TreeNode *leftResult = DFS(root->left, target);
    if (leftResult != nullptr)
    {
        return leftResult;
    }
    TreeNode *rightResult = DFS(root->right, target);
    return rightResult;
}