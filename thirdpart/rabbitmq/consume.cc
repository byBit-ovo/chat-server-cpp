#include "../common/rabbitmq.hpp"
#include <gflags/gflags.h>
#include "../common/logger.hpp"

DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");
//std::function<void(std::function<void(const Message &message, uint64_t deliveryTag, bool redelivered)>, 
//uint64_t deliveryTag, bool redelivered)>
using CallBack = std::function<void(const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)>;

void MessageCallBack(AMQP::TcpChannel* channel, const AMQP::Message &message,uint64_t deliveryTag, bool redelivered)
{
	std::string messa;
	messa.assign(message.body(),message.bodySize());
	std::cout << messa << std::endl;
	channel->ack(deliveryTag);
}
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


	// const std::string message;
	// AMQP::DeferredConsumer &defer3 = channel.consume("test-queue");
	// defer3.onError([](const char *message)
	// {
	// 	std::cout << "Consume fail!" << message << std::endl;
	// 	exit(0);
	// });
	// //std::function<void(std::function<void(const Message &message, uint64_t deliveryTag, bool redelivered)>, uint64_t deliveryTag, bool redelivered)>
	// auto call = std::bind(MessageCallBack,&channel,std::placeholders::_1,std::placehkolders::_2,std::placeholders::_3);
	// defer3.onReceived(call);
    // ev_run(loop, 0);
    google::ParseCommandLineFlags(&argc,&argv,true);

    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);

	MY_IM::MQClient mqclient("root","200533","127.0.0.1:5672");
	mqclient.DeclareComponents("test-exchange","test-queue");
	mqclient.Consume("test-queue",[](const char* data,size_t len){
		char buffer[len+1];
		memcpy(buffer,data,len);
		buffer[len]=0;
		std::cout << buffer << std::endl;
	});
	sleep(20);
	return 0;
}