#include <DB.h>
#include <TreeNode.h>
#include <list>
#include <prune.h>
#include <training.h>
#include <prune.h>
#include <utility.h>
#include <math.h>
#include <predict.h>
#include <limits.h>

TreeNode * prune(TreeNode * decision_tree){
  list<TreeNode *> tree_set;  //剪枝后的数集合
  tree_set.push_back(decision_tree);

  TreeNode * temp = decision_tree;
  while(1){
    temp = tree_copy(temp);
    TreeNode * node_to_delete = select_min_a_node(temp);
    if(node_to_delete == temp)
      break;
    tree_delete(node_to_delete);
    tree_set.push_back(temp);
  }

  TreeNode * best_tree = choose_best_tree(tree_set);

  //释放tree_set
  for(list<TreeNode *>::iterator iter = tree_set.begin();iter != tree_set.end(); ++iter){
    if(*iter == best_tree)
      continue;
    delete *iter;
  }
  return best_tree;
}

TreeNode * select_min_a_node(TreeNode * decision_tree){
  double min_a = 99999999;
  TreeNode * result;
  list<TreeNode *> tree_stack;
  tree_stack.push_front(decision_tree);
  while(tree_stack.size() > 0){
    TreeNode * current_node = tree_stack.front();
    tree_stack.pop_front();
    //如果是叶子节点，则直接跳过
    if(current_node->left_child == 0 && current_node->right_child == 0){
      continue;
    }

    if(current_node->a < min_a){
      result = current_node;
      min_a = current_node->a;
    }
    if(current_node->left_child != 0)
      tree_stack.push_front(current_node->left_child);
    if(current_node->right_child != 0)
      tree_stack.push_front(current_node->right_child);
  }
  return result;
}

TreeNode * tree_copy(TreeNode * decision_tree){
  return new TreeNode(decision_tree);
}

void tree_delete(TreeNode * & sub_tree){
  if(sub_tree->left_child != 0){
    delete sub_tree->left_child;
    sub_tree->left_child = 0;
  }
  if(sub_tree->right_child != 0){
    delete sub_tree->right_child;
    sub_tree->right_child = 0;
  }
  sub_tree->label = sub_tree->most_label;
}

TreeNode * choose_best_tree(list<TreeNode *> tree_set){
  list<TreeNode *> chosen_history;
  for(int i=0;i<1;i++){
    list<int> error; // 每一个树分错数量的集合
    list<int> ids = generate_random_data(0.1);  // 只用十分之一的局部数据进行剪枝测试
    DB * db_helper = new DB();
    for(list<TreeNode *>::iterator iter = tree_set.begin() ;iter != tree_set.end(); ++iter){
      int number_error = 0;
      for(list<int>::iterator iter_id = ids.begin();iter_id != ids.end(); ++iter_id){
        string predict_label = predict(*iter, *iter_id);
        string real_label = db_helper->query_string("select label from training_data where id = "
         + int2string(*iter_id));
        if(real_label != predict_label){
          number_error += 1;
        }
      }
      error.push_back(number_error);
    }

    int total = (int)(db_helper->select_count("select count(*) from training_data") *0.1);
    delete db_helper;

    int min_number_error = error.front();
    for(list<int>::iterator iter = error.begin();iter != error.end(); ++iter){
      if(*iter < min_number_error)
        min_number_error = *iter;
    }

    float se = sqrt(min_number_error * (total - min_number_error) / total);

    TreeNode * best_tree;
    int min_number_leaf = INT_MAX;
    list<int>::iterator iter_error = error.begin();
    for(list<TreeNode *>::iterator iter_tree = tree_set.begin()
                      ;iter_error != error.end(); ++iter_error,++iter_tree){
      if(*iter_error <= se + min_number_error && (*iter_tree)->number_leaf < min_number_leaf){
        best_tree = *iter_tree;
        min_number_leaf = best_tree->number_leaf;
      }
    }

    chosen_history.push_back(best_tree);
  }

  list<int> chosen_times;
  for(list<TreeNode *>::iterator iter = tree_set.begin();iter != tree_set.end(); ++iter){
    int chosen_time = 0;
    for(list<TreeNode *>::iterator iter_history = chosen_history.begin();iter_history != chosen_history.end(); ++iter_history){
      if((* iter_history) == *iter)
        chosen_time += 1;
    }
    chosen_times.push_back(chosen_time);
  }

  int max_chosen_time = 0;
  TreeNode * best_tree;
  list<int>::iterator iter_chosen_time = chosen_times.begin();
  for(list<TreeNode *>::iterator iter_tree = tree_set.begin();
    iter_chosen_time != chosen_times.end(); ++iter_chosen_time,++iter_tree){
    if(*iter_chosen_time > max_chosen_time){
      max_chosen_time = *iter_chosen_time;
      best_tree = *iter_tree;
    }
  }

  return best_tree;
}
