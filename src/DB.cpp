#include "DB.h"
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset_metadata.h>
#include <string>
#include <set>
#include <utility.h>
#include <map>

using namespace std;

DB::DB()
{
    this->driver = sql::mysql::get_mysql_driver_instance();
    this->con = this->driver->connect("tcp://127.0.0.1:3306/network_data", "root", "ccx1024cc");
}

DB::~DB()
{
    delete con;
}

sql::ResultSet * DB::query(const string & sql){
  sql::Statement * st = con->createStatement();
  sql::ResultSet * rs = st->executeQuery(sql);
  delete st;
  return rs;
}

int DB::select_count(const string &sql){
  sql::Statement * st = con->createStatement();
  sql::ResultSet * rs = st->executeQuery(sql);
  int num = 0;
  while(rs->next()){
    num = rs->getInt(1);
  }
  delete st;
  delete rs;
  return num;
}

list<int> DB::query_int_list(const string & sql){
  sql::Statement * st = con->createStatement();
  sql::ResultSet * rs = st->executeQuery(sql);
  list<int> result_list;
  while(rs->next()){
    result_list.push_back(rs->getInt(1));
  }
  delete rs;
  delete st;
  return result_list;
}

list<string> DB::query_string_list(const string & sql){
  sql::Statement * st = con->createStatement();
  sql::ResultSet * rs = st->executeQuery(sql);
  list<string> result_list;
  while(rs->next()){
    result_list.push_back(rs->getString(1));
  }
  delete rs;
  delete st;
  return result_list;
}

string DB::query_string(const string & sql){
  sql::Statement * st = con->createStatement();
  sql::ResultSet * rs = st->executeQuery(sql);
  string result;
  while(rs->next()){
    result = rs->getString(1);
  }
  delete rs;
  delete st;
  return result;
}

string DB::query_string(const string & sql,list<int> & ids){
  sql::Statement * st = con->createStatement();
  string sql_final = sql + list2str(ids);
  sql::ResultSet * rs = st->executeQuery(sql_final);
  string result;
  while(rs->next()){
    result = rs->getString(1);
  }
  delete rs;
  delete st;
  return result;
}

list<string> DB::query_string_list(const string & sql,list<int> & ids){
  sql::Statement * st = con->createStatement();
  string sql_final = sql + list2str(ids);
  sql::ResultSet * rs = st->executeQuery(sql_final);
  list<string> result;
  while(rs->next()){
    result.push_back(rs->getString(1));
  }
  delete rs;
  delete st;
  return result;
}

int DB::select_count(const string & sql,list<int> & ids){
  sql::Statement * st = con->createStatement();
  int result = 0;
  //分批求解
  list<int> temp;
  for(list<int>::iterator iter_id = ids.begin();iter_id != ids.end();++iter_id){
    if(temp.size() >= 1000){
      string sql_final = sql + list2str(temp);
      sql::ResultSet * rs = st->executeQuery(sql_final);
      while(rs->next()){
        result += rs->getInt(1);
      }
      delete rs;
      temp.clear();
    }else{
      temp.push_back(*iter_id);
    }
    //string sql_final = sql + int2string(*iter_id);
   // sql::ResultSet * rs = st->executeQuery(sql_final);
    //while(rs->next()){
     // if(rs->getInt(1) > 0){
     //   ++ result;
     // }
    //}
   // delete rs;
  }
  if(temp.size() > 0){
    string sql_final = sql + list2str(temp);
    sql::ResultSet * rs = st->executeQuery(sql_final);
    while(rs->next()){
      result += rs->getInt(1);
    }
    delete rs;
  }
  delete st;
  return result;
}

list<int> DB::query_int_list(const string &sql, list<int> &ids){
  sql::Statement * st = con->createStatement();
  string sql_final = sql + list2str(ids);
  sql::ResultSet * rs = st->executeQuery(sql_final);
  list<int> result;
  while(rs->next()){
    result.push_back(rs->getInt(1));
  }
  delete rs;
  delete st;
  return result;
}

list<float> DB::query_float_list(const string & sql, list<int> & ids){
  sql::Statement * st = con->createStatement();
  list<float> result;
  for(list<int>::iterator iter_id = ids.begin();iter_id != ids.end();++iter_id){
    string sql_final = sql + int2string(*iter_id);
    sql::ResultSet * rs = st->executeQuery(sql_final);
    while(rs->next()){
      result.push_back(atof(rs->getString(1).c_str()));
    }
    delete rs;
  }

  delete st;
  return result;
}

map<string,string> DB::select_map(string & sql){
  map<string,string> result;
  sql::Statement * st = con->createStatement();
  sql::ResultSet * rs = st->executeQuery(sql);
  sql::ResultSetMetaData * rs_meta = rs->getMetaData();

  while(rs->next()){
    for(unsigned int i=0;i<rs_meta->getColumnCount();i++){
      result[rs_meta->getColumnLabel(i+1)] = rs->getString(i+1);
    }
  }
  delete rs;
  delete st;
  return result;
}

int DB::update(const string & sql){
  sql::Statement * st = con->createStatement();
  int result = st->executeUpdate(sql);
  delete st;
  return result;
}

string DB::list2str(list<int> & ids){
  string str = " (";
  int  test = 0;
  for(list<int>::iterator iter = ids.begin();iter != ids.end(); iter++){
    if(iter == ids.begin()){
      str += int2string(*iter);
    }else{
      str += "," + int2string(*iter);
    }
  }
  str += ") ";
  return str;
}

string DB::select_most(const string & sql,list<int> &ids){  //label
  sql::Statement * st = con->createStatement();
  map<string,int> label_count;
  for(list<int>::iterator iter_id = ids.begin();iter_id != ids.end();++iter_id){
    sql::ResultSet * rs = st->executeQuery(sql + int2string(*iter_id));
    while(rs->next()){
      string label = rs->getString(1);
      if(label_count.find(label) == label_count.end()){
        label_count.find(label)->second += 1;
      }else{
        label_count[label] = 0;
      }
    }
    delete rs;
  }
  int max_count = -1;
  string result;
  for(map<string,int>::iterator iter_label_count = label_count.begin();iter_label_count != label_count.end();++iter_label_count){
    if(max_count < iter_label_count->second){
      max_count= iter_label_count->second;
      result = iter_label_count->first;
    }
  }

  delete st;
  return result;
}
