#include <brpc/server.h>
#include <butil/logging.h>
#include "main.pb.h"
#include <iostream>
#include "../common/etcd.hpp"
#include <gflags/gflags.h>
using std::cout;
using std::endl;
//继承EchoService创建子类，实现rpc调用业务
DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");
DEFINE_int64(ttl,3,"连接保活时长");
DEFINE_string(etcd_host,"http://127.0.0.1:2379","etcd服务端地址");
DEFINE_string(base_dir,"/service","服务监控根目录");
DEFINE_string(service_url,"game/shoot","射击游戏目录");
DEFINE_string(self_addr,"127.0.0.1:9000","Rpc服务提供地址");

class EchoService1: public example::EchoService
{
	public:
		virtual void 
		Echo(google::protobuf::RpcController* controller,const ::example::EchoRequest* request,
			 example::EchoResponse* response,::google::protobuf::Closure* done) override
		{
			brpc::ClosureGuard guard(done);
			// LOG_INFO("Recieve a msg: {}",request->message());
			cout << request->message() << endl;
			response->set_message(request->message() + "(by Server)");
		}

};


int main(int argc, char *argv[])
{
	google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);

	logging::LoggingSettings setting;
	setting.logging_dest = logging::LoggingDestination::LOG_TO_NONE;
	// close logger in brpc
	logging::InitLogging(setting);

	//implement one service,expose to outside
	brpc::Server server;
	EchoService1 service1;
	if(-1 == server.AddService(&service1,brpc::ServiceOwnership::SERVER_DOESNT_OWN_SERVICE))
	{
		LOG_ERROR("Add service error");
		return -1;
	}
	
	//start service server
	brpc::ServerOptions op;
	op.idle_timeout_sec = -1;
	op.num_threads = 1;
	if(server.Start(9000,&op)  == -1){
		LOG_ERROR("Server start error");
		return -1;
	} 

	// connect to etcd server
	Registerant::ptr rclient = std::make_shared<Registerant>(FLAGS_etcd_host,FLAGS_ttl);
	// register service addr to etcd server
	bool ret = rclient->Register(FLAGS_base_dir + "/echo/instance",FLAGS_self_addr);
	if(ret == true)
	{
		LOG_INFO("Service register successfully!({})",FLAGS_base_dir + "/echo/instance");
	}
	else{
		LOG_ERROR("Service register fail!({})",FLAGS_base_dir + "echo/instance");
	}
	//start listening
	server.RunUntilAskedToQuit();
	return 0;
}

