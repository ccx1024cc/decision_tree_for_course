#include <training.h>
#include <string>
#include <DB.h>
#include <cppconn/resultset.h>
#include <map>
#include <list>
#include <set>
#include <stdlib.h>
#include <sstream>
#include <utility.h>

using namespace std;

int level = 0;
int max_level = 8;

/**
 * 训练的入口函数
 *
 */
TreeNode * start_training(){
  //训练使用所有数据
  cout << "use all training data,loading all ids" << endl;
  DB * db_helper = new DB();
  list<int> ids = db_helper->query_int_list("select id from training_data");
  delete db_helper;
  cout << "   " <<ids.size() << " records"<<endl;

  list<int> attr_index;
  for(int i=0;i<NUM_ATTR;i++){
    attr_index.push_back(i);
  }

  cout << "initialize : " <<endl;
  cout << "   number of data : " << ids.size() <<endl;
  cout << "   number of attribution : " << attr_index.size() <<endl;
  return start_training_process(ids,attr_index);
}

/**
 * 训练的实际递归函数
 *
 */
TreeNode * start_training_process(list<int> & training_data, list<int> attr_index){
  level ++;
  cout << "===========================================================================" << endl;
  cout << "level " << int2string(level)<<endl;
  cout << "number of data : " << training_data.size() << " number of attribution : "
       << attr_index.size() << endl;

  //不存在数据
  if(training_data.size() == 0){
    cout << "no data in training data set,return NULL" << endl;
    level -- ;
    return 0;
  }

  //所有数据属于一个类
  cout << "checking is in same label" << endl;
  if(is_all_same_label(training_data)){
    DB * db_helper = new DB();
    string sql = "select label from training_data where id = " + int2string(*training_data.begin());
    string label = db_helper->query_string(sql);
    delete db_helper;
    TreeNode * leaf = new TreeNode("",0);
    leaf->label = label;
    leaf->number_leaf = 1;
    leaf->rt = 0;  // 误差为0
    leaf->rt_subtree = 0;  //子树误差也为0
    cout << "   all in same label : " <<label <<endl;
    cout << "   error : "<<leaf->rt << endl;
    cout << "   error of subtree : " << leaf->rt_subtree<<endl;
    cout << "   number of leaf : " <<leaf->number_leaf << endl;
    cout << "   return leaf : " << label <<endl;
    level -- ;
    return leaf;
  }
  cout << "   not in same label" <<endl;
  //如果没有属性，则返回大多数类标签的节点
  if(attr_index.size() == 0 || level >= max_level){
    if(level >= 5){
      cout << "level >= 5" << endl;
    }else{
      cout << "no attribution to split" << endl;
    }
    cout << "searching the most label" << endl;

    DB * db_helper = new DB();
    string sql = "select count(label),label from training_data where id in ";
    string sql_suffix = " group by label";
    string most_label = db_helper->select_most(sql,training_data,sql_suffix);

    TreeNode * leaf = new TreeNode("",0);
    leaf->label = most_label;
    sql = "select count(*) from training_data where label != '" + leaf->label + "' and id in ";
    int different_label_num = db_helper->select_count(sql,training_data);
    double total = db_helper->select_count("select count(*) from training_data");
    leaf->rt = different_label_num/total;
    leaf->rt_subtree = leaf->rt;  // 为了方便计算，虽然叶子节点子树误差应该为0,但是设置为节点误差
    leaf->number_leaf = 1;
    delete db_helper;

    cout << "most label : " << most_label << endl;
    cout << "error : "<<leaf->rt << endl;
    cout << "error of subtree : " << leaf->rt_subtree<<endl;
    cout << "number of leaf : " <<leaf->number_leaf << endl;
    cout << "return leaf : " << most_label << endl;
    level --;
    return leaf;
  }

  float min_gini = 999999;
  int best_attr_index = 0;
  float best_split_value = 0;  //如果是连续属性，则为值。如果是离散属性，则为下标
  //计算每个属性的基尼系数，找到分割属性和分割点
  cout << "calculating gini:" << endl;
  for(list<int>::iterator iter = attr_index.begin();iter != attr_index.end();++iter){
    list<float>  result = calculate_gini(training_data,attr_array[*iter]);
    if(min_gini > result.back()){
      min_gini = result.back();
      best_attr_index = *iter;
      best_split_value = result.front();
    }
  }

  cout << "-----------------------------------"<<endl;
  cout << "gini compution finished" << endl;
  cout << "   min gini = "<<double2string(min_gini)<<endl;

  //分割训练数据，分割属性
  DB * db_helper = new DB();
  string sql_base = "select id from training_data where " + attr_array[best_attr_index];
  string sql_value = double2string(best_split_value);

  //如果是标量，则求出真正的值
  if(best_attr_index == 1){ //protocol_type
    sql_value = "'" + protocol_array[(int)best_split_value] + "'";
  }else if(best_attr_index == 2){  //service
    sql_value = "'" + service_array[(int)best_split_value] + "'";
  }else if(best_attr_index == 3){ //flag
    sql_value = "'" + flag_array[(int)best_split_value] + "'";
  }
  cout << "best attribution : " << attr_array[best_attr_index] << endl;
  cout << " best split value : " << sql_value << endl;

  //分割数据
  cout << "spliting dataset" << endl;
  string sql1 = sql_base + " < " + sql_value + " and id in ";
  string sql2 = sql_base + " >= " + sql_value + " and id in ";
  list<int> sub_training_dataset1 = db_helper->query_int_list(sql1,training_data);
  list<int> sub_training_dataset2 = db_helper->query_int_list(sql2,training_data);

  string sql = "select count(label),label from training_data where id in ";
  string sql_suffix = " group by label";
  string most_label = db_helper->select_most(sql,training_data,sql_suffix);

  sql = "select count(*) from training_data where label != '" + most_label + "' and id in ";
  int difference_label_number = db_helper->select_count(sql,training_data);
  double total = db_helper->select_count("select count(*) from training_data");
  delete db_helper;
  training_data.clear();

  //删除属性
  cout << "deleting attribution " << attr_array[best_attr_index] <<" from attribution set" << endl;
  attr_index.remove(best_attr_index);

  cout << "generating sub-tree" << endl;
  //递归产生子树
  TreeNode * split_node = new TreeNode(attr_array[best_attr_index],best_split_value);
  split_node->left_child = start_training_process(sub_training_dataset1,attr_index);
  split_node->right_child = start_training_process(sub_training_dataset2,attr_index);
  cout << "finish tree build at node of attribution " << attr_array[best_attr_index] << endl;
  cout << "*************************************************************************"<<endl;
  cout << "error calculating" << endl;
  //统计误差

  split_node->most_label = most_label;
  split_node->rt = difference_label_number/total;
  split_node->rt_subtree = 0;
  split_node->rt_subtree += split_node->left_child != 0?split_node->left_child->rt_subtree:0;
  split_node->rt_subtree += split_node->right_child != 0 ?split_node->right_child->rt_subtree:0;
  split_node->number_leaf = 0;
  split_node->number_leaf += split_node->left_child != 0?split_node->left_child->number_leaf:0;
  split_node->number_leaf += split_node->right_child != 0?split_node->right_child->number_leaf:0;
  split_node->a = 0;
  if(split_node->number_leaf == 1){
    split_node->a += (split_node->rt - split_node->rt_subtree)/(split_node->number_leaf);
  }else{
    split_node->a += (split_node->rt - split_node->rt_subtree)/(split_node->number_leaf - 1);
  }

  cout << "error : "<<split_node->rt << endl;
  cout << "error of subtree : " << split_node->rt_subtree<<endl;
  cout << "number of leaf : " <<split_node->number_leaf << endl;
  cout << "cover error : " << double2string(split_node->a) << endl;
  cout << "return decision-node at attribution: " << split_node->split_attr << endl;
  level --;
  return split_node;
}

/**
 * 随机删除一定数量的测试数据id
 *
 */
list<int> generate_random_data(float factor){
    cout << "generate random data:" <<endl;

    DB * db_helper = new DB();
    int total_num = db_helper->select_count("select count(*) from training_data");
    delete db_helper;
    cout << "total number of data : "<< total_num << endl;

    unsigned int needed_num = (int)(total_num * factor);
    cout << "need " << needed_num << " ids" << endl;

    cout << "start generating random ids" << endl;
    map<int,int> exits;
    while(exits.size() < needed_num){
      int id = rand() % total_num + 1;
      exits.insert(pair<int,int>(id,1));
    }
    cout << "generation complete !" << endl;

    list<int> ids;
    for (map<int,int>::iterator iter = exits.begin();iter != exits.end();iter ++){
      ids.push_front(iter -> first);
    }
    exits.clear();
    return ids;
}

bool is_all_same_label(list<int> & ids){
  DB * db_helper = new DB();
  string sql = "select distinct label from training_data where id in ";
  list<string> label = db_helper->query_string_list(sql,ids);
  cout << "   number of label in data set : " <<label.size()  << endl;
  delete db_helper;
  if(label.size() > 1)
    return false;
  else
    return true;
}

list<float> calculate_gini(list<int> & ids,const string & split_attr){
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
  cout << "  calculate candidate for " << split_attr << endl;
  list<float> result;
  DB * db_helper = new DB();
  float min_gini = 10;
  float best_split_value;  //如果是标量属性，则代表最佳划分值的下标
  if(split_attr == "protocol_type"){
    cout << "   number of different value for " <<split_attr << " : "<<int2string(NUM_PROTOCOL) << endl;
    cout<<"   number of split value to try : "<<int2string(NUM_PROTOCOL)<<endl;
    for(int i=0;i<NUM_PROTOCOL;i++){
      cout << "-----------------------------------"<<endl;
      string split_value = protocol_array[i];
      cout << "   split value = "<<split_value<<endl;
      float num_subset1 = db_helper->select_count("select count(id) from training_data where protocol_type < '"
       + split_value + "' and id in ",ids);
      float num_subset2 = db_helper->select_count("select count(id) from training_data where protocol_type >= '"
       + split_value + "' and id in ",ids);
      cout<<"   sub-data set size1 : "<<int2string(num_subset1)<<endl;
      cout<<"   sub-data set size2 : "<<int2string(num_subset2)<<endl;
      float gini1 = 1;
      for(int j=0;j<NUM_LABELS && num_subset1 > 0;j++){
        string sql = "select count(id) from training_data where protocol_type < '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id in ";
        int num_label_in_subset1 = db_helper->select_count(sql,ids);
        gini1 -= (num_label_in_subset1 / num_subset1) * (num_label_in_subset1 / num_subset1);
      }
      float gini2 = 1;
      for(int j=0;j<NUM_LABELS && num_subset2 > 0;j++){
        string sql = "select count(id) from training_data where protocol_type >= '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id in ";
        int num_label_in_subset2 = db_helper->select_count(sql,ids);
        gini2 -= (num_label_in_subset2 / num_subset2) * (num_label_in_subset2 / num_subset2);
      }
      float temp_gini = (num_subset1/ids.size())*gini1 + (num_subset2/ids.size()) * gini2;
      cout << "   current gini = " <<double2string(temp_gini)<<endl;
      cout<< "   min gini = "<<double2string(min_gini)<<endl;
      if(min_gini > temp_gini){
        min_gini = temp_gini;
        best_split_value = i;
      }
    }
  }else if(split_attr == "service"){
    for(int i=0;i<NUM_SERVICE;i++){
      cout << "   number of different value for " <<split_attr << " : "<<int2string(NUM_SERVICE) << endl;
      cout<<"   number of split value to try : "<<int2string(NUM_SERVICE)<<endl;
      cout << "-----------------------------------"<<endl;
      string split_value = service_array[i];
      cout << "   split value = "<<split_value<<endl;
      float num_subset1 = db_helper->select_count("select count(id) from training_data where service < '"
       + split_value + "' and id in ",ids);
      float num_subset2 = db_helper->select_count("select count(id) from training_data where service >= '"
       + split_value + "' and id in ",ids);
      cout<<"   sub-data set size1 : "<<int2string(num_subset1)<<endl;
      cout<<"   sub-data set size2 : "<<int2string(num_subset2)<<endl;
      float gini1 = 1;
      for(int j=0;j<NUM_LABELS && num_subset1 > 0;j++){
        string sql = "select count(id) from training_data where service < '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id in ";
        int num_label_in_subset1 = db_helper->select_count(sql,ids);
        gini1 -= (num_label_in_subset1 / num_subset1) * (num_label_in_subset1 / num_subset1);
      }
      float gini2 = 1;
      for(int j=0;j<NUM_LABELS && num_subset2 > 0;j++){
        string sql = "select count(id) from training_data where service >= '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id in ";
        int num_label_in_subset2 = db_helper->select_count(sql,ids);
        gini2 -= (num_label_in_subset2 / num_subset2) * (num_label_in_subset2 / num_subset2);
      }
      float temp_gini = (num_subset1/ids.size())*gini1 + (num_subset2/ids.size()) * gini2;
      cout << "   current gini = " <<double2string(temp_gini)<<endl;
      cout<< "   min gini = "<<double2string(min_gini)<<endl;
      if(min_gini > temp_gini){
        min_gini = temp_gini;
        best_split_value = i;
      }
    }
  }else if(split_attr == "flag"){
    for(int i=0;i<NUM_FLAG;i++){
      cout << "   number of different value for " <<split_attr << " : "<<int2string(NUM_FLAG) << endl;
      cout<<"   number of split value to try : "<<int2string(NUM_FLAG)<<endl;
      cout << "-----------------------------------"<<endl;
      string split_value = flag_array[i];
      cout << "   split value = "<<split_value<<endl;
      float num_subset1 = db_helper->select_count("select count(id) from training_data where flag < '"
       + split_value + "' and id in ",ids);
      float num_subset2 = db_helper->select_count("select count(id) from training_data where flag >= '"
       + split_value + "' and id in ",ids);
      cout<<"   sub-data set size1 : "<<int2string(num_subset1)<<endl;
      cout<<"   sub-data set size2 : "<<int2string(num_subset2)<<endl;
      float gini1 = 1;
      for(int j=0;j<NUM_LABELS && num_subset1 > 0;j++){
        string sql = "select count(id) from training_data where flag < '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id in ";
        int num_label_in_subset1 = db_helper->select_count(sql,ids);
        gini1 -= (num_label_in_subset1 / num_subset1) * (num_label_in_subset1 / num_subset1);
      }
      float gini2 = 1;
      for(int j=0;j<NUM_LABELS && num_subset2 > 0;j++){
        string sql = "select count(id) from training_data where flag >= '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id in ";
        int num_label_in_subset2 = db_helper->select_count(sql,ids);
        gini2 -= (num_label_in_subset2 / num_subset2) * (num_label_in_subset2 / num_subset2);
      }
      float temp_gini = (num_subset1/ids.size())*gini1 + (num_subset2/ids.size()) * gini2;
      cout << "   current gini = " <<double2string(temp_gini)<<endl;
      cout<< "   min gini = "<<double2string(min_gini)<<endl;
      if(min_gini > temp_gini){
        min_gini = temp_gini;
        best_split_value = i;
      }
    }
  }else{
    list<float> values = db_helper->query_float_list("select DISTINCT "
    + split_attr + " from training_data where id in ", ids);
    values.sort();
    cout << "   number of different value for " <<split_attr << " : "<<values.size() << endl;

    list<float> split_candidate;
    list<float>::iterator last_iter = values.begin();
    for(list<float>::iterator iter = values.begin(); iter != values.end(); ++iter){
      if(iter == values.begin())
        continue;
      split_candidate.push_back((*iter + *last_iter)/2);
      last_iter = iter;
    }
    cout<<"   number of split value to try : "<<split_candidate.size()<<endl;

    if(split_candidate.size() == 0){
      cout <<"   no split value !" <<endl;
      best_split_value = values.front();
      result.push_back(best_split_value);
      result.push_back(min_gini);
      return result;
    }

    for(list<float>::iterator iter = split_candidate.begin(); iter != split_candidate.end(); ++iter){
      float split_value = *iter;
      string split_value_in_string = double2string(split_value);
      cout << "-----------------------------------"<<endl;
      cout << "   split value = "<<split_value_in_string<<endl;
      float num_subset1 = db_helper->select_count("select count(id) from training_data where " + split_attr
       + " < " + split_value_in_string + " and id in ",ids);
      float num_subset2 = db_helper->select_count("select count(id) from training_data where " + split_attr
       + " >= " + split_value_in_string + " and id in ",ids);
      cout<<"   sub-data set size1 : "<<int2string(num_subset1)<<endl;
      cout<<"   sub-data set size2 : "<<int2string(num_subset2)<<endl;
      float gini1 = 1;
      for(int j=0;j<NUM_LABELS && num_subset1 > 0;j++){
        string sql = "select count(id) from training_data where " + split_attr + " < " + split_value_in_string +
        " and label = '" + LABEL_ARRAY[j] + "' and id in ";
        int num_label_in_subset1 = db_helper->select_count(sql,ids);
        gini1 -= (num_label_in_subset1 / num_subset1) * (num_label_in_subset1 / num_subset1);
      }
      float gini2 = 1;
      for(int j=0;j<NUM_LABELS && num_subset2 > 0;j++){
        string sql = "select count(id) from training_data where " + split_attr + " >= " + split_value_in_string +
        " and label = '" + LABEL_ARRAY[j] + "' and id in ";
        int num_label_in_subset2 = db_helper->select_count(sql,ids);
        gini2 -= (num_label_in_subset2 / num_subset2) * (num_label_in_subset2 / num_subset2);
      }
      float temp_gini = (num_subset1/ids.size())*gini1 + (num_subset2/ids.size()) * gini2;
      cout << "   current gini = " <<double2string(temp_gini)<<endl;
      cout<< "   min gini = "<<double2string(min_gini)<<endl;
      if(min_gini > temp_gini){
        min_gini = temp_gini;
        best_split_value = split_value;
      }
    }
  }
  delete db_helper;
  result.push_back(best_split_value);
  result.push_back(min_gini);
  return result;
}
