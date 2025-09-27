#include "file_server.hpp"
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
DEFINE_string(instance_name, "/file_service/instance", "当前实例名称");
DEFINE_string(access_addr,"127.0.0.1:9292","Rpc服务至注册中心地址");


DEFINE_int32(listen_port, 9292, "Rpc服务器监听端口");
DEFINE_int32(rpc_timeout, -1, "Rpc调用超时时间");
DEFINE_int32(rpc_threads, 1, "Rpc的IO线程数量");

int main(int argc, char *argv[])
{
	google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);

	logging::LoggingSettings setting;
	setting.logging_dest = logging::LoggingDestination::LOG_TO_NONE;
	// close logger in brpc
	logging::InitLogging(setting);

	MY_IM::FileServerBuilder file_builder;
	file_builder.
	Construct_File_Service().
	Construct_Rpc_Server(FLAGS_listen_port,FLAGS_rpc_timeout,FLAGS_rpc_threads).
	Construct_Register(FLAGS_etcd_host,FLAGS_base_dir + FLAGS_instance_name,FLAGS_access_addr);

	auto file_server = file_builder.Build();
	file_server->Start();
	return 0;
}

