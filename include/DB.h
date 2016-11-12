#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <list>
#include <map>

using namespace std;

class DB
{
    public:
        DB();
        virtual ~DB();
        int select_count(const string & sql);
        sql::ResultSet * query(const string & sql);
        list<int> query_int_list(const string & sql);
        list<int> query_int_list(const string & sql, list<int> & ids);
        list<string> query_string_list(const string & sql);
        string query_string(const string & sql);
        string query_string(const string & sql,list<int> & ids);
        list<string> query_string_list(const string &sql,list<int> & ids);
        int select_count(const string & sql,list<int> & ids);
        string list2str(list<int> & ids);
        list<float> query_float_list(const string & sql, list<int> & ids);
        map<string,string> select_map(const string & sql);
        int update(const string & sql);
        string select_most(const string & sql,list<int> &ids,const string & sql_suffix);
        int insert(const string & sql);

        sql::Connection * con;

    protected:

    private:
        sql::mysql::MySQL_Driver * driver;

};
