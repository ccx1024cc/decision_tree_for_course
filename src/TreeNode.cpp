#include "TreeNode.h"
#include <string>
#include <DB.h>
#include <map>
#include <utility.h>

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

TreeNode::TreeNode(int id_on_disk){
  DB * db_helper = new DB();
  map<string,string> temp = db_helper->select_map("select * from tree_node where id = " + int2string(id_on_disk));
  delete db_helper;

  this->id_on_disk = string2int(temp["id"]);
  this->a = string2double(temp["a"]);
  this->label = temp["label"];
  this->most_label = temp["most_label"];
  this->number_leaf = string2int(temp["number_leaf"]);
  this->rt = string2double(temp["rt"]);
  this->rt_subtree = string2double(temp["rt_subtree"]);
  this->split_attr = temp["split_attr"];
  this->split_value = string2double(temp["split_value"]);
  this->left_child_id_on_disk = string2int(temp["left_child"]);
  this->right_child_id_on_disk = string2int(temp["right_child"]);

  if(this->left_child_id_on_disk != -1){
    this->left_child = new TreeNode(this->left_child_id_on_disk);
  }else{
    this->left_child = 0;
  }
  if(this->right_child_id_on_disk != -1){
    this->right_child = new TreeNode(this->right_child_id_on_disk);
  }else{
    this->right_child = 0;
  }
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

int TreeNode::save_on_disk(){
  if(this->left_child != 0){
    int id = this->left_child->save_on_disk();
    this->left_child_id_on_disk = id;
  }else{
    this->left_child_id_on_disk = -1;
  }

  if(this->right_child != 0){
    int id = this->right_child->save_on_disk();
    this->right_child_id_on_disk = id;
  }else{
    this->right_child_id_on_disk = -1;
  }

  DB * db_helper = new DB();
  string sql = "insert into tree_node(split_attr,split_value,";
  sql += "label,most_label,rt,rt_subtree,number_leaf,a,left_child,right_child) values(";
  sql += "'"+this->split_attr+"'," + double2string(this->split_value)+",'"+this->label+"','";
  sql += this->most_label + "'," + double2string(this->rt) + "," + double2string(this->rt_subtree);
  sql += "," + int2string(this->number_leaf) + "," + double2string(this->a) + "," ;
  sql += int2string(this->left_child_id_on_disk) + "," + int2string(this->right_child_id_on_disk);
  sql += ")";
  int id = db_helper->insert(sql);
  this->id_on_disk = id;
  delete db_helper;
  return id;
}
