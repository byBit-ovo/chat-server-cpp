#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Value.hpp>
#include <string>
#include <functional>
#include <memory>
#include "logger.hpp"


class Discoverer{
	public:
		using call_back_t = std::function<void(const std::string&,const std::string&)>;
		using ptr = std::shared_ptr<Discoverer>;
		Discoverer() = default;
		Discoverer(const std::string &host,const std::string &base_dir,
		const call_back_t &put_call,const call_back_t &del_call):
		_client(std::make_shared<etcd::Client>(host)),
		_put_call(put_call),_del_call(del_call)
		{
			auto resp = _client->ls(base_dir).get();
			if(resp.is_ok() == false)
			{
				LOG_ERROR("Service dicover failed: {}",resp.error_message());
			}
			int len = resp.keys().size();
			for(int i=0;i<len;++i)
			{
				LOG_INFO("{} is providing {}", resp.value(i).as_string(),resp.key(i));
			}
			auto call_back = std::bind(&Discoverer::CallBack,this,std::placeholders::_1);
			_watcher = std::make_shared<etcd::Watcher>(*_client,base_dir,call_back,true);
			_watcher->Wait();
		}
	private:
		void CallBack(etcd::Response resp)
		{
			if(resp.is_ok() == false)
			{
				LOG_ERROR("Response error!");
				return;
			}
			for(auto const &event: resp.events())
			{
				if(event.event_type() == etcd::Event::EventType::PUT){
					LOG_INFO("A put event: {}:{}->{}",event.kv().key(),event.prev_kv().as_string(),event.kv().as_string());
					if(_put_call)
					{
						_put_call(event.kv().key(),event.kv().as_string());
					}
				}
				else if(event.event_type() == etcd::Event::EventType::DELETE_)
				{
					LOG_INFO("A delete event: {}: {}",event.kv().key(),event.prev_kv().as_string())
					if(_del_call)
					{
						_del_call(event.kv().key(),event.prev_kv().as_string());
					}
				}
			}
		}
	private:
		std::shared_ptr<etcd::Client> _client;
		std::shared_ptr<etcd::Watcher> _watcher;
		call_back_t _put_call;
		call_back_t _del_call;
};


class Registerant{
	public:
		using ptr = std::shared_ptr<Registerant>;
		Registerant(const std::string &host,int64_t ttl):
		_client(std::make_shared<etcd::Client>(host)),_ttl(ttl)
		{
			_keep_alive = _client->leasekeepalive(_ttl).get();
			_lease_id = _keep_alive->Lease();
			
		}
		bool Register(const std::string &key,const std::string &item)
		{
			auto resp = _client->put(key,item,_lease_id).get();
			if(resp.is_ok() == false)
			{
				LOG_ERROR("Put fail: {}",resp.error_message());
				return false;
			}
			return true;
		}
	private:
		std::shared_ptr<etcd::Client> _client;
		std::shared_ptr<etcd::KeepAlive> _keep_alive;
		int64_t _ttl;
		int64_t _lease_id;
};