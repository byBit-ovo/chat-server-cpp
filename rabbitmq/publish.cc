#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <openssl/ssl.h>
#include <openssl/opensslv.h>



int main()
{
	// access to the event loop
    auto *loop = EV_DEFAULT;
    // handler for libev
    AMQP::LibEvHandler handler(loop);

    // make a connection
    AMQP::Address address("amqp://root:200533@127.0.0.1:15672/");
	// AMQP::Address address("amqps://guest:guest@localhost/");
    AMQP::TcpConnection connection(&handler, address);
    
    // we need a channel too
    AMQP::TcpChannel channel(&connection);

	AMQP::Deferred &defer = channel.declareExchange("test-exchange",AMQP::ExchangeType::direct);
	defer.onError([](const char *message)
	{
		std::cout << "Declare Exchange fail!" << message << std::endl;
	});
	
	return 0;
}