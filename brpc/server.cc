#include <brpc/server.h>
#include <butil/logging.h>
#include "main.pb.h"
#include <iostream>
using std::cout;
using std::endl;
//继承EchoService创建子类，实现rpc调用业务
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
	logging::LoggingSettings setting;
	setting.logging_dest =  logging::LoggingDestination::LOG_TO_NONE;
	logging::InitLogging(setting);
	brpc::Server server;
	EchoService1 service1;
	if(-1 == server.AddService(&service1,brpc::ServiceOwnership::SERVER_DOESNT_OWN_SERVICE))
	{
		// LOG_ERROR("Add service error");

		return -1;
	}
	brpc::ServerOptions op;
	op.idle_timeout_sec = -1;
	op.num_threads = 1;
	if(server.Start(9000,&op)  == -1){
		// LOG_ERROR("Server start error");
		return -1;
	} 
	server.RunUntilAskedToQuit();
	return 0;
}

