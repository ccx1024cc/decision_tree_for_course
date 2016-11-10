#ifndef TREENODE_H
#define TREENODE_H
#include <string>

using namespace std;

class TreeNode
{
    public:
        TreeNode(string split_attr,float split_value);
        TreeNode(TreeNode * node);
        virtual ~TreeNode();

        string split_attr;
        float split_value;
        string label;
        string most_label; //集合中最多的标签
        TreeNode * left_child;  // left_child->split_value < this->split_value
        TreeNode * right_child;

        double rt;  // 节点误差
        double rt_subtree;  // 子树误差
        int number_leaf;  // 叶子节点个数
        double a;  //表面误差率增益
    protected:

    private:
};

#endif
