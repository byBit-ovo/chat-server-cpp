#include <cpr/cpr.h>
#include <elasticlient/client.h>
#include <cpr/response.h>
#include <iostream>
#include "logger.hpp"
#include "../json/json.hpp"

class EsIndex
{
	public:
		using Client = std::shared_ptr<elasticlient::Client>;
		EsIndex(const Client &client,const std::string &index_name,
		const std::string &type):_client(client),_name(index_name),_type(type)
		{
			Json::Value analysis;
			Json::Value analyzer;
			Json::Value ik;
			Json::Value tokenizer;
			tokenizer["tokenizer"] = "ik_max_word";
            ik["ik"] = tokenizer;
            analyzer["analyzer"] = ik;
            analysis["analysis"] = analyzer;
            _index["settings"] = analysis;
			Json::Value mappings;
			mappings["dynamic"] = true;
		}
		EsIndex& Append(const std::string &key, const std::string &type = "text",
			const std::string &analyzer = "ik_max_word",bool enabled = true)
		{
			Json::Value fields;
            fields["type"] = type;
            fields["analyzer"] = analyzer;
            if (enabled == false ) fields["enabled"] = enabled;
            _properties[key] = fields;
            return *this;
		}
		bool create(const std::string &id)
		{
			Json::Value mappings;
			mappings["dynamic"] = true;
			mappings["properties"] = _properties;
			_index["mappings"] = mappings;
			std::string req = JsonUtil::serialize(_index);
			if(req.empty())
			{
				LOG_INFO("Json requeset deserialize fail!");
				return false;
			}
			LOG_INFO("request: {}",req);
			try {
				//create index to es
                auto rsp = _client->index(_name, _type, id, req);
                if (rsp.status_code < 200 || rsp.status_code >= 300) {
                    LOG_ERROR("Creating ES index {} fail,response code: {}", _name, rsp.status_code);
                    return false;
                }
				std::cout << rsp.status_code << std::endl;
				std::cout << rsp.text << std::endl;
            } catch(std::exception &e) {
                LOG_ERROR("Creating ES index {} fail: {}", _name, e.what());
                return false;
            }
			
			return true;
		}
		std::string serialzie(){
			return JsonUtil::serialize(_index);
		}
	private:
		std::string _name;
		std::string _type;
		Json::Value _index;
		Json::Value _properties;
		Client _client;
};

class EsInsert
{

};

