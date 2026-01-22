#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>
#include <string>
using std::cout;
using std::endl;
int main(int argc,char *argv[])
{
	etcd::Client client("http://127.0.0.1:2379");
	auto keep_alive = client.leasekeepalive(10).get();
	int64_t lease_id = keep_alive->Lease();
	auto resp = client.put("/service/game","hostip:7890",lease_id).get();
	if(resp.is_ok() == false)
	{
		cout << "Put fail" << endl;
		return -1;
	}
	resp = client.put("/service/music","hostip:7891").get();
	if(resp.is_ok() == false)
	{
		cout << "Put fail" << endl;
		return -1;
	}
	resp = client.put("/service/sport","hostip:7890").get();
	if(resp.is_ok() == false)
	{
		cout << "Put fail" << endl;
		return -1;
	}
	sleep(30);
	return 0;
}