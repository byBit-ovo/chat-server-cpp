#include <brpc/channel.h>
#include <iostream>
#include <butil/logging.h>
#include "main.pb.h"
using std::cout;
using std::endl;
int main(int argc, char *argv[])
{
	brpc::Channel channel;
	brpc::ChannelOptions op;
	op.connect_timeout_ms = -1;
	op.timeout_ms = -1;
	op.max_retry = 3;
	op.protocol = "baidu_std";
	channel.Init("127.0.0.1:9000",&op);

	example::EchoService_Stub stub(&channel);
	example::EchoRequest *req = new example::EchoRequest();
	example::EchoResponse *resp = new example::EchoResponse();
	req->set_message("hello,bybit");
	brpc::Controller *cntl = new brpc::Controller();
	stub.Echo(cntl,req,resp,nullptr);
	if(cntl->Failed() == true){
		return -1;
	}
	cout << resp->message() << endl;
	delete req;
	delete resp;
	delete cntl;
	return 0;
}

