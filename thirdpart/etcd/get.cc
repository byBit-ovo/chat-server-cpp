#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Value.hpp>
#include <string>
using std::cout;
using std::endl;
void CallBack(etcd::Response resp)
{
	if(resp.is_ok()){
		cout << resp.value().as_string() << "is ok" << endl;
	}
	else
	{
		cout << resp.value().as_string() << "is not ok" << endl;
	}

	for(auto const &event: resp.events())
	{
		if(event.event_type() == etcd::Event::EventType::PUT){
			cout << "A put event:" << event.kv().key() << ": " <<  
			event.kv().as_string() << "-->" << event.prev_kv().as_string() << endl;
		}
		else if(event.event_type() == etcd::Event::EventType::DELETE_)
		{
			cout << "A delete event: " << event.kv().key() << ": " << event.prev_kv().as_string() << endl; 
		}
	}
}
int main(int argc,char *argv[])
{
	etcd::Client client("http://127.0.0.1:2379");
	auto resp = client.ls("/service").get();
	if(resp.is_ok() == false)
	{
		cout << "Get fail" << endl;
		return -1;
	}
	int len = resp.keys().size();
	for(int i=0;i<len;++i)
	{
		cout << resp.value(i).as_string() << " has puted a " << resp.key(i) << " service"<< endl;
	}
	etcd::Watcher watcher(client,"/service",CallBack,true);
	watcher.Wait();
	return 0;
}