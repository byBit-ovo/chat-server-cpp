#pragma once

#include <brpc/channel.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "logger.hpp"
#include <unordered_set>
// a manager of all channels for one specific service 


namespace MY_IM
{
	class ServiceChannels{
	public:
		using Ptr = std::shared_ptr<ServiceChannels>;
		using ChannelPtr = std::shared_ptr<brpc::Channel>;
		ServiceChannels(const std::string &name):_service(name),_choose_index(0)
		{}
		~ServiceChannels(){}

		ChannelPtr choose(){
			if(_channels.empty()){
				LOG_WARNING("No hosts can provide {}", _service);
				return ServiceChannels::ChannelPtr();
			}
			ChannelPtr choice = _channels[_choose_index];
			++_choose_index;
			_choose_index %= _channels.size();
			return choice;
		}
		// 
		void AddChannel(const std::string &host)
		{
			ChannelPtr channel = std::make_shared<brpc::Channel>();
			brpc::ChannelOptions op;
			op.connect_timeout_ms = -1;
			op.timeout_ms = -1;
			op.max_retry = 3;
			op.protocol = "baidu_std";
			if(channel->Init(host.c_str(),&op) == -1){
				LOG_ERROR("Channel({} by {}) init error", _service, host);
				return;
			}
			std::unique_lock<std::mutex> guard(_mutex);
			_channels.push_back(channel);
			_channel.insert(std::make_pair(host,channel));
		}

		void RemoveChannel(const std::string &host)
		{
			std::unique_lock<std::mutex> guard(_mutex);
			auto iter = _channel.find(host);
			if(iter == _channel.end()){
				LOG_WARNING("{} host doesn't exist in service({})", host,_service);
				return;
			}
			auto it = _channels.begin();
			while(it != _channels.end())
			{
				if(*it == iter->second){
					_channels.erase(it);
					break;
				}
			}
			_channel.erase(host);
		}
	private:
		// which channel to choose
		size_t _choose_index;						
		// single specific service name
		std::string _service;
		// all channels for this service
		std::vector<ChannelPtr> _channels;
		// pointer to channel
		std::unordered_map<std::string, ChannelPtr> _channel;
		std::mutex _mutex;
};

// manager for all services
// cooperate with Discoverer
class ServiceManager
{
	public:
		using ptr = std::shared_ptr<ServiceManager>;
		// get certain channel of service
		ServiceChannels::ChannelPtr GetChannel(const std::string &name)
		{
			std::unique_lock<std::mutex> guard(_mutex);
			auto iter = _channels.find(name);
			if(iter == _channels.end())
			{
				LOG_WARNING("No hosts can provide {}", name);
				return ServiceChannels::ChannelPtr();
			}
			return iter->second->choose();
		}
		void FollowOn(const std::string &name)
		{
			std::unique_lock<std::mutex> guard(_mutex);
			_follows.insert(name);
		}
		// this method to discoverer's del_call
		void OfflineCall(const std::string &service_instance,const std::string &host)
		{
			std::string service_name = GetServiceName(service_instance); 
			ServiceChannels::Ptr channels;
			{
				std::unique_lock<std::mutex> guard(_mutex);

				auto ext = _follows.find(service_name);
				if(ext == _follows.end())
				{
					LOG_INFO("service({}) is not followed by,ignore",service_name);
					return;
				}

				channels = _channels.find(service_name)->second;
				if(channels.get() == nullptr)
				{
					LOG_WARNING("No manager for this service(OfflineCall): {}", service_name);
					return;
				}

			}
			channels->RemoveChannel(host);
			LOG_INFO("A host({}) is offline service({})",host,service_name);
		}
		// this method to discoverer's put_call
		void OnlineCall(const std::string &service_instance,const std::string &host)
		{
			// LOG_DEBUG("OnlineCall...");
			//a/b/c/instance --->.  a/b/c
			std::string service_name = GetServiceName(service_instance); 
			ServiceChannels::Ptr channels;
			{
				std::unique_lock<std::mutex> guard(_mutex);

				auto ext = _follows.find(service_name);
				if(ext == _follows.end())
				{
					LOG_INFO("service({}) is not followed by,ignore",service_name);
					return;
				}
				auto iter = _channels.find(service_name);
				if(iter == _channels.end())
				{
					_channels.insert(std::make_pair(service_name,std::make_shared<ServiceChannels>(service_name)));
					channels = _channels[service_name];
				}
				else
				{
					channels = iter->second;					
				}

			}
			// LOG_DEBUG("OnlineCall...over");
			channels->AddChannel(host);
			LOG_INFO("A new host{} is online,service ({})",host,service_name);
		}
		private:
			std::string GetServiceName(const std::string &service_instance)
			{
				std::size_t pos = service_instance.rfind('/');
				if(pos == std::string::npos){
					return "";
				}
				// /a/b/instance
				return service_instance.substr(0,pos);
			}
	private:
		std::mutex _mutex;
		// which services this manager follows
		std::unordered_set<std::string> _follows;
		// which channels/hosts are providing this service
		std::unordered_map<std::string,ServiceChannels::Ptr> _channels;
};

}