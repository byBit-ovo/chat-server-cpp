#include <brpc/channel.h>
#include <gflags/gflags.h>
#include <iostream>
#include <butil/logging.h>
#include "main.pb.h"
#include <thread>
#include "../common/channel.hpp"
#include "../common/etcd.hpp"
#include "../common/logger.hpp"
using std::cout;
using std::endl;
DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");
DEFINE_string(etcd_host,"http://127.0.0.1:2379","etcd服务端地址");
DEFINE_string(base_dir,"/service","服务监控根目录");
DEFINE_string(echo_service,"/echo","Rpc请求的服务");
int main(int argc, char *argv[])
{
	google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
	ServiceManager::Ptr manager = std::make_shared<ServiceManager>();
	manager->FollowOn(FLAGS_base_dir + FLAGS_echo_service);

	Discoverer::call_back_t put_call = std::bind(&ServiceManager::OnlineCall,manager.get(),
	std::placeholders::_1,std::placeholders::_2);
	Discoverer::call_back_t del_call = std::bind(&ServiceManager::OfflineCall,manager.get(),
	std::placeholders::_1,std::placeholders::_2);
	
	Discoverer::ptr dicoverer_client = std::make_shared<Discoverer>(FLAGS_etcd_host,FLAGS_base_dir,put_call,del_call);

	// brpc::Channel channel;
	// brpc::ChannelOptions op;
	// op.connect_timeout_ms = -1;
	// op.timeout_ms = -1;
	// op.max_retry = 3;
	// op.protocol = "baidu_std";
	// channel.Init("127.0.0.1:9000",&op);
	
	while(1)
	{
		auto channel = manager->GetChannel(FLAGS_base_dir + FLAGS_echo_service);
		if(channel.get() == nullptr)
		{
			LOG_ERROR("No accesssible channel for service->{}",FLAGS_base_dir + FLAGS_echo_service);
			sleep(1);
			continue;
		}
		LOG_DEBUG("Get a channel successfully!");
		example::EchoService_Stub stub(channel.get());
		example::EchoRequest req; 
		example::EchoResponse resp;
		req.set_message("hello,bybit");
		brpc::Controller cntl;
		LOG_DEBUG("ready to call Rpc-Echo!");
		stub.Echo(&cntl,&req,&resp,nullptr);
		if(cntl.Failed() == true){
			LOG_ERROR("Rpc call failed!({})",cntl.ErrorText());
			sleep(1);
			continue;
		}
		
		cout << "Recieved a message: " << resp.message() << endl;
		sleep(1);
		
	}

	return 0;
}

