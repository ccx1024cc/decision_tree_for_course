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
  cout << "===========================================================================" << endl;
  cout << "number of data : " << training_data.size() << " number of attribution : "
       << attr_index.size() << endl;

  //不存在数据
  if(training_data.size() == 0){
    cout << "no data in training data set,return NULL" << endl;
    return 0;
  }

  //所有数据属于一个类
  cout << "checking is in same label?" << endl;
  if(is_all_same_label(training_data)){
    DB * db_helper = new DB();
    string label = db_helper->query_string("select label from training_data where id = "
                                            + * training_data.begin());
    delete db_helper;
    TreeNode * leaf = new TreeNode(0,0);
    leaf->label = label;
    leaf->number_leaf = 1;
    leaf->rt = 0;  // 误差为0
    leaf->rt_subtree = 0;  //子树误差也为0
    cout << "   all in same label : " <<label <<endl;
    cout << "   error : "<<leaf->rt << endl;
    cout << "   error of subtree : " << leaf->rt_subtree<<endl;
    cout << "   number of leaf : " <<leaf->number_leaf << endl;
    cout << "   return leaf : " << label <<endl;
    return leaf;
  }
  cout << "   not in same label" <<endl;
  //如果没有属性，则返回大多数类标签的节点
  if(attr_index.size() == 0){
    cout << "no attribution to split" << endl;
    cout << "searching the most label" << endl;

    DB * db_helper = new DB();
    string sql = "select label from training_data where id = ";
    string most_label = db_helper->select_most(sql,training_data);

    TreeNode * leaf = new TreeNode(0,0);
    leaf->label = most_label;
    sql = "select count(*) from training_data where label != '" + leaf->label + "' and id = ";
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
    return leaf;
  }

  float min_gini = 999999;
  int best_attr_index = 0;
  float best_split_value = 0;  //如果是连续属性，则为值。如果是离散属性，则为下标
  //计算每个属性的基尼系数，找到分割属性和分割点
  cout << "calculating gini:" << endl;
  for(list<int>::iterator iter = attr_index.begin();iter != attr_index.end();++iter){
    if(iter == attr_index.begin())
      continue;
    list<float>  result = calculate_gini(training_data,attr_array[*iter]);
    if(min_gini > result.back()){
      min_gini = result.back();
      best_attr_index = *iter;
      best_split_value = result.front();
    }
  }
  cout << "gini compution finished" << endl;

  //分割训练数据，分割属性
  DB * db_helper = new DB();
  string sql_base = "select id from training_data where " + attr_array[best_attr_index];
  ostringstream oss;
  oss << best_split_value;
  string sql_value = oss.str();
  oss.str("");

  //如果是标量，则求出真正的值
  if(best_attr_index == 2){ //protocol_type
    sql_value = "'" + protocol_array[(int)best_split_value] + "'";
  }else if(best_attr_index == 3){  //service
    sql_value = "'" + service_array[(int)best_split_value] + "'";
  }else if(best_attr_index == 4){ //flag
    sql_value = "'" + flag_array[(int)best_split_value] + "'";
  }
  cout << "best attribution : " << attr_array[best_attr_index] << endl;
  cout << " best split value : " << sql_value << endl;

  //分割数据
  cout << "spliting dataset" << endl;
  string sql1 = sql_base + " < " + sql_value;
  string sql2 = sql_base + " >= " + sql_value;
  list<int> sub_training_dataset1 = db_helper->query_int_list(sql1,training_data);
  list<int> sub_training_dataset2 = db_helper->query_int_list(sql2,training_data);
  delete db_helper;
  training_data.clear(); //清空内存

  //删除属性
  cout << "deleting attribution " << attr_array[best_attr_index] <<" from attribution set" << endl;
  attr_index.remove(best_attr_index);

  cout << "generating sub-tree" << endl;
  //递归产生子树
  TreeNode * split_node = new TreeNode(attr_array[best_attr_index],best_split_value);
  split_node->left_child = start_training_process(sub_training_dataset1,attr_index);
  split_node->right_child = start_training_process(sub_training_dataset2,attr_index);
  cout << "finish tree build at node of attribution " << attr_array[best_attr_index] << endl;

  cout << "error calculating" << endl;
  //统计误差
  db_helper = new DB();
  string sql = "select label from training_data where id = ";
  string most_label = db_helper->select_most(sql,training_data);

  sql = "select count(*) from training_data where label != '" + most_label + "' and id = ";
  int difference_label_number = db_helper->select_count(sql,training_data);
  double total = db_helper->select_count("select count(*) from training_data");
  delete db_helper;

  split_node->most_label = most_label;
  split_node->rt = difference_label_number/total;
  split_node->rt_subtree = 0;
  split_node->rt_subtree += split_node->left_child != 0?split_node->left_child->rt_subtree:0;
  split_node->rt_subtree += split_node->right_child != 0 ?split_node->right_child->rt_subtree:0;
  split_node->number_leaf = 0;
  split_node->number_leaf += split_node->left_child != 0?split_node->left_child->number_leaf:0;
  split_node->number_leaf += split_node->right_child != 0?split_node->right_child->number_leaf:0;
  split_node->a += (split_node->rt - split_node->rt_subtree)/(split_node->number_leaf - 1);

  cout << "error : "<<split_node->rt << endl;
  cout << "error of subtree : " << split_node->rt_subtree<<endl;
  cout << "number of leaf : " <<split_node->number_leaf << endl;
  cout << " 表面误差增益 : " << split_node->a << endl;
  cout << "return decision-node at attribution: " << split_node->split_attr << endl;
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
  //set<string> label;
  //for(list<int>::iterator iter_id = ids.begin();iter_id != ids.end();++iter_id){
  //  string final_sql = sql + int2string(*iter_id);
  //  label.insert(db_helper->query_string(final_sql));
  //}
  //cout << "number of label : " <<label.size()  << endl;
  list<string> label = db_helper->query_string_list(sql,ids);
  cout << label.size() <<endl;
  label.unique();
  cout << "number of label : " <<label.size()  << endl;
  delete db_helper;
  if(label.size() > 1)
    return false;
  else
    return true;
}

list<float> calculate_gini(list<int> & ids,const string & split_attr){
  cout << "  calculate candidate for " << split_attr << endl;
  list<float> result;
  DB * db_helper = new DB();
  float min_gini = 10;
  float best_split_value;  //如果是标量属性，则代表最佳划分值的下标
  if(split_attr == "protocol_type"){
    for(int i=0;i<NUM_PROTOCOL;i++){
      string split_value = protocol_array[i];
      int num_subset1 = db_helper->select_count("select count(id) from training_data where protocol_type < '"
       + split_value + "' and id = ",ids);
      int num_subset2 = db_helper->select_count("select count(id) from training_data where protocol_type >= '"
       + split_value + "' and id = ",ids);
      float gini1 = 1;
      for(int j=0;j<NUM_LABELS && num_subset1 > 0;j++){
        string sql = "select count(id) from training_data where protocol_type < '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id = ";
        int num_label_in_subset1 = db_helper->select_count(sql,ids);
        gini1 -= (num_label_in_subset1 / num_subset1) * (num_label_in_subset1 / num_subset1);
      }
      float gini2 = 1;
      for(int j=0;j<NUM_LABELS && num_subset2 > 0;j++){
        string sql = "select count(id) from training_data where protocol_type >= '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id = ";
        int num_label_in_subset2 = db_helper->select_count(sql,ids);
        gini2 -= (num_label_in_subset2 / num_subset2) * (num_label_in_subset2 / num_subset2);
      }
      float temp_gini = (num_subset1/ids.size())*gini1 + (num_subset2/ids.size()) * gini2;
      if(min_gini > temp_gini){
        min_gini = temp_gini;
        best_split_value = i;
      }
    }
  }else if(split_attr == "service"){
    for(int i=0;i<NUM_SERVICE;i++){
      string split_value = service_array[i];
      int num_subset1 = db_helper->select_count("select count(id) from training_data where service < '"
       + split_value + "' and id = ",ids);
      int num_subset2 = db_helper->select_count("select count(id) from training_data where service >= '"
       + split_value + "' and id = ",ids);
      float gini1 = 1;
      for(int j=0;j<NUM_LABELS && num_subset1 > 0;j++){
        string sql = "select count(id) from training_data where service < '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id = ";
        int num_label_in_subset1 = db_helper->select_count(sql,ids);
        gini1 -= (num_label_in_subset1 / num_subset1) * (num_label_in_subset1 / num_subset1);
      }
      float gini2 = 1;
      for(int j=0;j<NUM_LABELS && num_subset2 > 0;j++){
        string sql = "select count(id) from training_data where service >= '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id = ";
        int num_label_in_subset2 = db_helper->select_count(sql,ids);
        gini2 -= (num_label_in_subset2 / num_subset2) * (num_label_in_subset2 / num_subset2);
      }
      float temp_gini = (num_subset1/ids.size())*gini1 + (num_subset2/ids.size()) * gini2;
      if(min_gini > temp_gini){
        min_gini = temp_gini;
        best_split_value = i;
      }
    }
  }else if(split_attr == "flag"){
    for(int i=0;i<NUM_FLAG;i++){
      string split_value = flag_array[i];
      int num_subset1 = db_helper->select_count("select count(id) from training_data where flag < '"
       + split_value + "' and id = ",ids);
      int num_subset2 = db_helper->select_count("select count(id) from training_data where flag >= '"
       + split_value + "' and id = ",ids);
      float gini1 = 1;
      for(int j=0;j<NUM_LABELS && num_subset1 > 0;j++){
        string sql = "select count(id) from training_data where flag < '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id = ";
        int num_label_in_subset1 = db_helper->select_count(sql,ids);
        gini1 -= (num_label_in_subset1 / num_subset1) * (num_label_in_subset1 / num_subset1);
      }
      float gini2 = 1;
      for(int j=0;j<NUM_LABELS && num_subset2 > 0;j++){
        string sql = "select count(id) from training_data where flag >= '" + split_value +
        "' and label = '"+LABEL_ARRAY[j]+"' and id = ";
        int num_label_in_subset2 = db_helper->select_count(sql,ids);
        gini2 -= (num_label_in_subset2 / num_subset2) * (num_label_in_subset2 / num_subset2);
      }
      float temp_gini = (num_subset1/ids.size())*gini1 + (num_subset2/ids.size()) * gini2;
      if(min_gini > temp_gini){
        min_gini = temp_gini;
        best_split_value = i;
      }
    }
  }else{
    list<float> values = db_helper->query_float_list("select DISTINCT "
    + split_attr + " from training_data where id = ", ids);
    //去重
    set<float> different_values;
    for(list<float>::iterator iter_values = values.begin();iter_values != values.end();++iter_values){
      different_values.insert(*iter_values);
    }
    values.clear();
    for(set<float>::iterator iter_values = different_values.begin();iter_values != different_values.end();++iter_values){
      values.push_back(*iter_values);
    }
    values.sort();
    cout << "   number of different value for " <<split_attr << " : "<<values.size() << endl;

    list<float> split_candidate;
    list<float>::iterator last_iter = values.begin();
    for(list<float>::iterator iter = values.begin(); iter != values.end(); ++iter){
      if(iter == values.begin())
        continue;
      split_candidate.push_back((*iter * *last_iter)/2);
      last_iter = iter;
    }
    cout<<"   number of split value to try : "<<split_candidate.size()<<endl;

    for(list<float>::iterator iter = split_candidate.begin(); iter != split_candidate.end(); ++iter){
      float split_value = *iter;
      ostringstream oss;
      oss << split_value;
      string split_value_in_string = oss.str();
      oss.str("");
      cout<<"   split value to try : "<<split_value_in_string<<endl;
      int num_subset1 = db_helper->select_count("select count(id) from training_data where " + split_attr
       + " < " + split_value_in_string + " and id = ",ids);
      int num_subset2 = db_helper->select_count("select count(id) from training_data where " + split_attr
       + " >= " + split_value_in_string + " and id = ",ids);
      cout<<"   sub-data set size1 : "<<int2string(num_subset1)<<endl;
      cout<<"   sub-data set size2 : "<<int2string(num_subset2)<<endl;
      float gini1 = 1;
      for(int j=0;j<NUM_LABELS && num_subset1 > 0;j++){
        string sql = "select count(id) from training_data where " + split_attr + " < " + split_value_in_string +
        " and label = '" + LABEL_ARRAY[j] + "' and id = ";
        int num_label_in_subset1 = db_helper->select_count(sql,ids);
        gini1 -= (num_label_in_subset1 / num_subset1) * (num_label_in_subset1 / num_subset1);
      }
      cout<<"   gini1 : "<<gini1<<endl;
      float gini2 = 1;
      for(int j=0;j<NUM_LABELS && num_subset2 > 0;j++){
        string sql = "select count(id) from training_data where " + split_attr + " >= " + split_value_in_string +
        " and label = '" + LABEL_ARRAY[j] + "' and id = ";
        int num_label_in_subset2 = db_helper->select_count(sql,ids);
        gini2 -= (num_label_in_subset2 / num_subset2) * (num_label_in_subset2 / num_subset2);
      }
      cout<<"   gini2 : "<<gini2<<endl;
      float temp_gini = (num_subset1/ids.size())*gini1 + (num_subset2/ids.size()) * gini2;
      if(min_gini > temp_gini){
        min_gini = temp_gini;
        best_split_value = split_value;
      }
    }
  }
  result.push_back(best_split_value);
  result.push_back(min_gini);
  return result;
}
