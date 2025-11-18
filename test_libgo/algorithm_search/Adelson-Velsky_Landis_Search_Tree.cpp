#include<iostream>
#include<vector>
using namespace std;

/*
    AVL（Adelson-Velsky and Landis）自平衡二叉搜索树实现（带详细注释）
    注意：
        - 为了保持与原始代码行为一致，下面保留了原来函数名（leftRotate/rightRotate）但实现上
            它们的实际“旋转方向”与名字可能不一致。注释中会指出这一点，使用时请留意。
        - 已修正原代码中一处多余的反斜杠导致的语法错误（原来在 leftRotate 的一行末尾有 '\'）。
*/

/* AVL 树节点定义 */
struct AVLNode{
        int val;         // 节点存储的键值
        AVLNode *left;   // 左子节点指针
        AVLNode *right;  // 右子节点指针
        int height;      // 节点高度（叶子节点高度为 1）
        AVLNode(int x):val(x),left(nullptr),right(nullptr),height(1){}
};

/* 获取节点高度（空指针高度为 0） */
int getHeight(AVLNode* node){
        if(node==nullptr){
                return 0;
        }
        return node->height;
}

/* 计算节点的平衡因子：左子树高度 - 右子树高度
     平衡因子 > 1 表示左高，< -1 表示右高 */
int getBalanceFactor(AVLNode* node){
        if(node==nullptr){
                return 0;
        }
        return getHeight(node->left)-getHeight(node->right);
}

/*
    右旋（注意：函数名为 leftRotate，但实现是对 node 做“右旋”操作）
    旋转结构（旋转前，以 node 为根）：
                node                 rootChild
             /    \      ->       /    \
     rootChild  C           A    node
        /   \                      / \
     A     B                    B   C

    具体步骤：
        - 令 rootChild = node->left
        - 将 rootChild->right (B) 连接为 node->left
        - 将 node 作为 rootChild 的右子节点
        - 更新 node 和 rootChild 的高度
    返回旋转后新的根（rootChild）
*/
AVLNode* leftRotate(AVLNode* node){
        // 实际上是对 node 做右旋
        AVLNode* rootChild = node->left;
        AVLNode* rootChildRight = rootChild->right;

        // 调整指针：rootChild 变为新的根
        rootChild->right = node;
        node->left = rootChildRight;

        // 更新高度：先更新被降下的 node，然后更新新的根 rootChild
        node->height = max(getHeight(node->left), getHeight(node->right)) + 1;
        rootChild->height = max(getHeight(rootChild->left), getHeight(rootChild->right)) + 1;

        return rootChild;
}

/*
    左旋（注意：函数名为 rightRotate，但实现是对 node 做“左旋”操作）
    旋转结构（旋转前，以 node 为根）：
             node                 rootChild
            /    \      ->       /    \
         A   rootChild      node    C
                 /   \         /  \
                B     C       A    B

    具体步骤：
        - 令 rootChild = node->right
        - 将 rootChild->left (B) 连接为 node->right
        - 将 node 作为 rootChild 的左子节点
        - 更新高度并返回新的根 rootChild
*/
AVLNode* rightRotate(AVLNode* node){
        // 实际上是对 node 做左旋
        AVLNode* rootChild = node->right;
        AVLNode* rootChildLeft = rootChild->left;

        // 调整指针：rootChild 变为新的根
        rootChild->left = node;
        node->right = rootChildLeft;

        // 更新高度：先更新被降下的 node，然后更新新的根 rootChild
        node->height = max(getHeight(node->left), getHeight(node->right)) + 1;
        rootChild->height = max(getHeight(rootChild->left), getHeight(rootChild->right)) + 1;

        return rootChild;
}

/* 先对子节点做一次与父方向相反的旋转，再对父做一次与子方向相同的旋转
     leftRightRotate：先对左子节点做左旋（函数名 rightRotate），再对父做右旋（函数名 leftRotate）
     适用于左-右不平衡（Left-Right）情况 */
AVLNode* leftRightRotate(AVLNode* node){
        node->left = rightRotate(node->left); // 先对左子节点做“左旋”（rightRotate）
        return leftRotate(node);              // 再对当前节点做“右旋”（leftRotate）
}

/* 与上面对称：
     rightLeftRotate：先对右子节点做右旋（函数名 leftRotate），再对父做左旋（函数名 rightRotate）
     适用于右-左不平衡（Right-Left）情况 */
AVLNode* rightLeftRotate(AVLNode* node){
        node->right = leftRotate(node->right); // 先对右子节点做“右旋”（leftRotate）
        return rightRotate(node);               // 再对当前节点做“左旋”（rightRotate）
}

/*
    向 AVL 树插入值 val，并保持平衡
    参数：
        - root: 引用传递的根指针（注意：函数签名使用 AVLNode* &root）
        - val: 要插入的键值
    返回：
        - 插入后该子树的根指针

    算法步骤：
        1. 按二叉搜索树的规则递归插入节点。
        2. 回溯时更新当前节点高度。
        3. 计算平衡因子并根据 4 种不平衡情况执行相应旋转：
             - LL（左-左）：balanceFactor > 1 && val < root->left->val -> 右旋（leftRotate）
             - RR（右-右）：balanceFactor < -1 && val > root->right->val -> 左旋（rightRotate）
             - LR（左-右）：balanceFactor > 1 && val > root->left->val -> 先左旋左子树再右旋（leftRightRotate）
             - RL（右-左）：balanceFactor < -1 && val < root->right->val -> 先右旋右子树再左旋（rightLeftRotate）
*/
AVLNode* insertAVL(AVLNode* &root,int val){
        // 1) 标准 BST 插入
        if(root==nullptr){
                root=new AVLNode(val);
                return root;
        }
        if(val<root->val){
                root->left = insertAVL(root->left, val);
        }
        else{
                root->right = insertAVL(root->right, val);
        }

        // 2) 更新高度
        root->height = max(getHeight(root->left), getHeight(root->right)) + 1;

        // 3) 计算平衡因子并判断是否需要旋转
        int balanceFactor = getBalanceFactor(root);

        // LL 情况（左子树比右子树高，且新插入节点位于左子树的左侧）
        if(balanceFactor > 1 && val < root->left->val){
                return leftRotate(root); // 实现为右旋
        }
        // RR 情况（右子树比左子树高，且新插入节点位于右子树的右侧）
        if(balanceFactor < -1 && val > root->right->val){
                return rightRotate(root); // 实现为左旋
        }
        // LR 情况（左-右）
        if(balanceFactor > 1 && val > root->left->val){
                return leftRightRotate(root);
        }
        // RL 情况（右-左）
        if(balanceFactor < -1 && val < root->right->val){
                return rightLeftRotate(root);
        }

        // 若平衡则直接返回当前根
        return root;
}
/* 在 AVL 树中搜索值 val
    参数：
        - root: 当前子树根节点
        - val: 要搜索的键值
    返回：
        - 找到则返回对应节点指针，否则返回 nullptr
*/
AVLNode* searchAVL(AVLNode* root,int val){
        if(root==nullptr||root->val==val){
                return root;
        }
        if(val<root->val){
                return searchAVL(root->left,val);
        }
        else{
                return searchAVL(root->right,val);
        }
}
