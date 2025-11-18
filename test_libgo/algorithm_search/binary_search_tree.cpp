#include<iostream>
#include<vector>
using namespace std;
struct TreeNode{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x):val(x),left(nullptr),right(nullptr){}
};

void createBST(){
    return ;
}

TreeNode* insertBST(TreeNode* &root,int val){
    if(root==nullptr){
        root=new TreeNode(val);
        return root;
    }
    if(val<root->val){
        root->left=insertBST(root->left,val);
    }
    else{
        root->right=insertBST(root->right,val);
    }
}
TreeNode* searchBST(TreeNode* root,int val){
    if(root==nullptr||root->val==val){
        return root;
    }
    if(val<root->val){
        searchBST(root->left,val);
    }
    else{
        searchBST(root->right,val);
    }
}