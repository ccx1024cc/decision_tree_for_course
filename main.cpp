#include <iostream>
#include <TreeNode.h>
#include <training.h>
#include <predict.h>
#include <prune.h>
#include <utility.h>

using namespace std;

int main()
{
//    TreeNode * decision_tree = start_training();
//    decision_tree->save_on_disk();
//    cout<<"****************************************************"<<endl;
//    cout << "tree ID before prune : "<<int2string(decision_tree->id_on_disk)<<endl;


//    TreeNode * decision_tree = new TreeNode(23);
//    cout << "prune for node 23"<<endl;
//    decision_tree = prune(decision_tree);
//    decision_tree->save_on_disk();
//    cout << "tree ID after prune : "<<int2string(decision_tree->id_on_disk)<<endl;

    TreeNode * decision_tree = new TreeNode(98);
    predict(decision_tree);
    return 0;
}
