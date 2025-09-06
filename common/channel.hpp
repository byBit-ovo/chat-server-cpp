#include <brpc/channel.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "logger.hpp"

// a manager of all channels for one specific service 
class ServiceChannels{
	public:
		using Ptr = std::shared_ptr<ServiceChannels>;
		using ChannelPtr = std::shared_ptr<brpc::Channel>;
		ServiceChannels(const std::string &name):_service(name),_choose_index(0)
		{}
		~ServiceChannels(){}

		ChannelPtr choose(){
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

//

class ServiceManager
{

};