#include "TreeNode.h"
#include <string>

using namespace std;

TreeNode::TreeNode(string split_attr,float split_value)
{
    this->split_attr = split_attr;
    this->split_value = split_value;
    this->left_child = 0;
    this->right_child = 0;
}

TreeNode::TreeNode(TreeNode * node){
  this->a = node->a;
  this->label = node->label;
  this->most_label = node->most_label;
  this->number_leaf = node->number_leaf;
  this->rt = node->rt;
  this->rt_subtree = node->rt_subtree;
  this->split_attr = node->split_attr;
  this->split_value = node->split_value;
  if(node->left_child == 0)
    this->left_child = 0;
  else
    this->left_child = new TreeNode(node->left_child);
  if(node->right_child == 0)
    this->right_child = 0;
  else
    this->right_child = new TreeNode(node->right_child);
}

TreeNode::~TreeNode()
{
    if(this->left_child != 0){
      delete this->left_child;
      this->left_child = 0;
    }
    if(this->right_child != 0){
      delete this->right_child;
      this->right_child = 0;
    }
}
