#include "user_server.hpp"
#include <gflags/gflags.h>
using std::cout;
using std::endl;

DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");
DEFINE_int64(ttl,3,"连接保活时长");

DEFINE_string(etcd_host,"http://127.0.0.1:2379","etcd服务端地址");

DEFINE_string(base_dir,"/service","服务监控根目录");
DEFINE_string(service_name,"/user_service","用户服务名称");
DEFINE_string(instance_name, "/user_service/instance", "当前用户服务实例名称");

DEFINE_string(access_addr,"127.0.0.1:9190","Rpc服务至注册中心地址");

DEFINE_string(es_host, "http://127.0.0.1:9200/", "ES搜索引擎服务器URL");

DEFINE_int32(listen_port, 9190, "Rpc服务器监听端口");
DEFINE_int32(rpc_timeout, -1, "Rpc调用超时时间");
DEFINE_int32(rpc_threads, 1, "Rpc的IO线程数量");

DEFINE_string(mysql_host, "127.0.0.1", "Mysql服务器访问地址");
DEFINE_string(mysql_user, "root", "Mysql服务器访问用户名");
DEFINE_string(mysql_pswd, "123456", "Mysql服务器访问密码");
DEFINE_string(mysql_db, "bite_im", "Mysql默认库名称");
DEFINE_string(mysql_cset, "utf8", "Mysql客户端字符集");
DEFINE_int32(mysql_port, 0, "Mysql服务器访问端口");
DEFINE_int32(mysql_pool_count, 4, "Mysql连接池最大连接数量");

DEFINE_string(redis_host, "127.0.0.1", "Redis服务器访问地址");
DEFINE_int32(redis_port, 6379, "Redis服务器访问端口");
DEFINE_int32(redis_db, 0, "Redis默认库号");
DEFINE_bool(redis_keep_alive, true, "Redis长连接保活选项");

int main(int argc, char *argv[])
{
	google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);

	logging::LoggingSettings setting;
	setting.logging_dest = logging::LoggingDestination::LOG_TO_NONE;
	// close logger in brpc
	logging::InitLogging(setting);

	auto user_server_builder = std::make_shared<MY_IM::UserServerBuilder>();

	auto user_server = user_server_builder->
	Construct_Discover_Client(FLAGS_etcd_host,FLAGS_base_dir,FLAGS_service_name).
	Construct_Es_Client({FLAGS_es_host}).
	Construct_MySQL_Client(FLAGS_mysql_user,FLAGS_mysql_pswd,FLAGS_mysql_host,FLAGS_mysql_db,
		FLAGS_mysql_cset,FLAGS_mysql_port,FLAGS_mysql_pool_count).
	Construct_Redis_Client(FLAGS_redis_host,FLAGS_redis_port,FLAGS_redis_db,FLAGS_redis_keep_alive).
	Construct_Register(FLAGS_etcd_host,FLAGS_base_dir+FLAGS_instance_name,FLAGS_access_addr).
	Construct_Rpc_Server(FLAGS_listen_port,FLAGS_ttl,FLAGS_rpc_threads).
	Build();

	user_server->Start();
	return 0;
}

