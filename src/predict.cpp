#include <DB.h>
#include <predict.h>
#include <list>
#include <training.h>
#include <sstream>
#include <map>
#include <string>
#include <utility.h>

using namespace std;

void predict(TreeNode * decision_tree){
  DB * db_helper = new DB();
  list<int> ids = db_helper->query_int_list("select id from predict_data");
  for(list<int>::iterator iter = ids.begin();iter != ids.end();++iter){
    string label = predict(decision_tree,*iter);
    db_helper->update("update predict_data set label = " + label);
  }
  delete db_helper;
}

string predict(TreeNode * decision_tree,int id){
  cout<<"-----------------------------------------"<<endl;
  cout << "predicting for id:" << id << endl;
  TreeNode * current_node = decision_tree;
  string label;
  DB * db_helper = new DB();
  while(1){
    if(current_node == 0){
      label = "undefine";
      break;
    }
    if(current_node->left_child == 0 && current_node->right_child == 0){ //叶子节点，直接判定
      label = current_node->label;
      break;
    }

    bool left; // 是否是左子树

    cout<<"----------------------------------------------------"<<endl;
    string split_attr = current_node->split_attr;
    if(current_node->split_attr == "protocol_type"){
      string split_value = protocol_array[(int)current_node->split_value];
      string real_value = db_helper->query_string("select protocol_type from training_data where id = "
      + int2string(id));
      if(real_value < split_value){
        left = true;
      }else{
        left = false;
      }
    }else if(current_node->split_attr == "service"){
      string split_value = service_array[(int)current_node->split_value];
      string real_value = db_helper->query_string("select service from training_data where id = "
       + int2string(id));
      if(real_value < split_value){
        left = true;
      }else{
        left = false;
      }
    }else if(current_node->split_attr == "flag"){
      string split_value = flag_array[(int)current_node->split_value];
      string real_value = db_helper->query_string("select flag from training_data where id = "
      + int2string(id));
      if(real_value < split_value){
        left = true;
      }else{
        left = false;
      }
    }else{
      float split_value = current_node->split_value;
      string temp = db_helper->query_string("select " + split_attr
      + " from training_data where id = " + int2string(id));
      float real_value = (float)atof(temp.c_str());
      if(real_value < split_value){
        left = true;
      }else{
        left = false;
      }
      string split_value_in_string;
      ostringstream oss;
      oss << split_value;
      split_value_in_string = oss.str();
      oss.str("");
    }

    current_node = left?current_node->left_child:current_node->right_child;
  }

  delete db_helper;
  return label;
}
