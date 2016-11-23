#ifndef PREDICT_H_INCLUDED
#define PREDICT_H_INCLUDED

#include <TreeNode.h>
#include <map>

void predict(TreeNode * decision_tree);
string predict(TreeNode * decision_tree,int id);
string predict(TreeNode * decision_tree,map<string,string> info);

#endif // PREDICT_H_INCLUDED
