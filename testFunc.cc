#include <string>
#include <iostream>
#include "json/json.hpp"

std::string GetServiceName(const std::string &service_instance)
{
	std::size_t pos = service_instance.rfind('/');
	if(pos = std::string::npos){// ==woshishabi
		return "";
	}
	// /a/b/instance
	return service_instance.substr(0,pos);
}
class Animal
{
	public:
		virtual void Echo()
		{
			std::cout << "Animal" << std::endl;
		}
		virtual ~Animal()
		{
			std::cout << "~Animal" << std::endl;
		}
};
class Dog:public Animal
{
	public:
		virtual void Echo()override
		{
			std::cout << "Woof" << std::endl;
		}
		virtual ~Dog(){
			std::cout << "~Dog()" << std::endl;

		}
};


int main()
{
	// Animal *animal = new Dog();
	// animal->Echo();
	// delete animal;
	Json::Value root;
	root["num1"] =1;
	std::string ret = JsonUtil::serialize(root);
	std::cout << ret << std::endl;
	return 0;
}