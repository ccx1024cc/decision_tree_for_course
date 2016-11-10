#include <TreeNode.h>
#include <list>

using namespace std;

const int NUM_ATTR = 32;
const string attr_array[] = {"id","duration","protocol_type","service"
,"flag","src_bytes","dst_bytes","land","wrong_fragment","urgent","hot"
,"num_failed_logins","logged_in","num_compromised","root_shell","su_attempted"
,"num_root","num_file_creations","num_shells","num_access_files","num_outbound_cmds"
,"is_host_login","is_guest_login","count","srv_count","serror_rate","srv_serror_rate"
,"rerror_rate","srv_rerror_rate","same_srv_rate","diff_srv_rate","srv_diff_host_rate"};

const int NUM_FLAG = 11;
const string flag_array[] = {"SF","S1","REJ","S2","S0","S3","RSTO","RSTR","RSTOS0","OTH","SH"};

const int NUM_SERVICE = 66;
const string service_array[] = {"http","smtp","finger","domain_u"
,"auth","telnet","ftp","eco_i","ntp_u","ecr_i","other","private","pop_3"
,"ftp_data","rje","time","mtp","link","remote_job","gopher","ssh","name"
,"whois","domain","login","imap4","daytime","ctf","nntp","shell","IRC"
,"nnsp","http_443","exec","printer","efs","courier","uucp","klogin"
,"kshell","echo","discard","systat","supdup","iso_tsap","hostnames"
,"csnet_ns","pop_2","sunrpc","uucp_path","netbios_ns","netbios_ssn"
,"netbios_dgm","sql_net","vmnet","bgp","Z39_50","ldap","netstat"
,"urh_i","X11","urp_i","pm_dump","tftp_u","tim_i","red_i"};

const int NUM_PROTOCOL = 3;
const string protocol_array[] = {"tcp","udp","icmp"};

const int NUM_LABELS = 23;
const string LABEL_ARRAY[] = {"back","buffer_overflow","ftp_write"
,"guess_passwd","imap","ipsweep","land","loadmodule","multihop"
,"neptune","nmap","normal","perl","phf","pod","portsweep","rootkit"
,"satan","smurf","spy","teardrop","warezclient","warezmaster"};

TreeNode * start_training();

list<float> calculate_gini(list<int> & ids,const string & split_attr);
bool is_all_same_label(list<int> & ids);
list<int> generate_random_data(float factor);
TreeNode * start_training_process(list<int> & training_data, list<int> attr_index);

