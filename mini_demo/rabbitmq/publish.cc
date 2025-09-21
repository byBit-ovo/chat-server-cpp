#include "../common/rabbitmq.hpp"
#include "../common/logger.hpp"
#include <gflags/gflags.h>

DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");
int main(int argc, char *argv[])
{
	// // access to the event loop
    // auto *loop = EV_DEFAULT;
    // // handler for libev
    // AMQP::LibEvHandler handler(loop);

    // // make a connection
    // AMQP::Address address("amqp://root:200533@127.0.0.1:5672/");
	// // AMQP::Address address("amqps://guest:guest@localhost/");
    // AMQP::TcpConnection connection(&handler, address);
    
    // // we need a channel too
    // AMQP::TcpChannel channel(&connection);

	// AMQP::Deferred &defer = channel.declareExchange("test-exchange",AMQP::ExchangeType::direct);
	// defer.onError([](const char *message)
	// {
	// 	std::cout << "Declare Exchange fail!" << message << std::endl;
	// 	exit(0);
	// });
	// defer.onSuccess([](){
	// 	std::cout << "Exchange create successfully!" << std::endl;
	// });

	// AMQP::DeferredQueue &queue = channel.declareQueue("test-queue");
	// queue.onError([](const char *message)
	// {
	// 	std::cout << "Declare queue fail!" << message << std::endl;
	// 	exit(0);

	// });
	// queue.onSuccess([](){
	// 	std::cout << "queue create successfully!" << std::endl;
	// });

	// AMQP::Deferred &defer2 = channel.bindQueue("test-exchange","test-queue","test-queue-key");
	// defer2.onError([](const char *message)
	// {
	// 	std::cout << "Bind fail!" << message << std::endl;
	// 	exit(0);
	// });
	// defer2.onSuccess([](){
	// 	std::cout << "Bind successfully!" << std::endl;
	// });

	// for(int i=0;i < 10;++i)
	// {
	// 	const std::string message = "Hello,this is a test-message-" + std::to_string(i);
	// 	bool ret = channel.publish("test-exchange","test-queue-key",message);
	// 	if(ret==false)
	// 	{
	// 		std::cout << "Publish fail!" << std::endl;
	// 	}
	// }
    // ev_run(loop, 0);
    google::ParseCommandLineFlags(&argc,&argv,true);

    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
	
	MY_IM::MQClient mqclient("root","200533","127.0.0.1:5672");
	mqclient.DeclareComponents("test-exchange","test-queue");
	std::string mess = "This is a test message";
	for(int i=0;i < 10;++i)
	{
		mqclient.Publish("test-exchange",mess + std::to_string(i));
		// sleep(1);
	}
	sleep(1);
	return 0;
}