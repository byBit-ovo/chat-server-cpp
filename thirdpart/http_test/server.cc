#include <httplib.h>


int main()
{
	httplib::Server server;
	server.Get("/test",[](const httplib::Request& req,httplib::Response& res){
		std::cout << req.method << std::endl;
		std::cout << req.path << std::endl;
		std::string body = "This is a test";
		res.content_length_ = body.size();
		res.status = 200;
		res.set_header("Name:","bybit");
		res.set_header("Age:","13");
		res.set_header("Gender:","male");
		res.set_header("keep-alive","60");
		res.set_content(body,"text");
		
	});
	server.listen("0.0.0.0",9000);
	return 0;
}