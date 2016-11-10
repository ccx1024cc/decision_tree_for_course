#ifndef PRUNE_H_INCLUDED
#define PRUNE_H_INCLUDED

#include <TreeNode.h>
#include <list>

using namespace std;

TreeNode * prune(TreeNode * decision_tree);
TreeNode * select_min_a_node(TreeNode * decision_tree);
TreeNode * tree_copy(TreeNode * decision_tree);
void tree_delete(TreeNode * & sub_tree);  // 删除一棵树中的子树
TreeNode * choose_best_tree(list<TreeNode *> tree_set);
#endif // PRUNE_H_INCLUDED
