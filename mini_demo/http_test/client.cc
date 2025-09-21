#include <httplib.h>


int main()
{
	httplib::Client client("http://127.0.0.1:9000");
	auto res = client.Get("/test");
	std::cout << res.value().body << std::endl;
	for(auto &header:res.value().headers)
	{
		std::cout << header.first << ": " << header.second << std::endl;
	}
	
	return 0;
}