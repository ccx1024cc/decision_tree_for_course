#include <iostream>
#include <TreeNode.h>
#include <training.h>
#include <predict.h>
#include <prune.h>

using namespace std;

int main()
{
    TreeNode * decision_tree = start_training();
    decision_tree = prune(decision_tree);
    predict(decision_tree);
    return 0;
}
