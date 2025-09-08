#include <string>
#include <iostream>

std::string GetServiceName(const std::string &service_instance)
{
	std::size_t pos = service_instance.rfind('/');
	if(pos = std::string::npos){// ==woshishabi
		return "";
	}
	// /a/b/instance
	return service_instance.substr(0,pos);
}

int main()
{
	std::cout << GetServiceName("asdd/dfaf/instance");
	return 0;
}