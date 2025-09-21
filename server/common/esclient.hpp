#pragma once

#include <cpr/cpr.h>
#include <elasticlient/client.h>
#include <cpr/response.h>
#include <iostream>
#include "logger.hpp"
#include "../json/json.hpp"

namespace MY_IM
{
	class EsIndex
	{
	public:
		using Client = std::shared_ptr<elasticlient::Client>;
		EsIndex(const Client &client, const std::string &index_name,
				const std::string &type) : _client(client), _name(index_name), _type(type)
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
		EsIndex &Append(const std::string &key, const std::string &type = "text",
						const std::string &analyzer = "ik_max_word", bool enabled = true)
		{
			Json::Value fields;
			fields["type"] = type;
			fields["analyzer"] = analyzer;
			if (enabled == false)
				fields["enabled"] = enabled;
			_properties[key] = fields;
			return *this;
		}
		bool create(const std::string &id = "")
		{
			Json::Value mappings;
			mappings["dynamic"] = true;
			mappings["properties"] = _properties;
			_index["mappings"] = mappings;
			std::string req = JsonUtil::serialize(_index);
			if (req.empty())
			{
				LOG_INFO("Json requeset deserialize fail!");
				return false;
			}
			// LOG_INFO("request: {}",req);
			try
			{
				// create index to es
				auto rsp = _client->index(_name, _type, id, req);
				if (rsp.status_code < 200 || rsp.status_code >= 300)
				{
					LOG_ERROR("Creating ES index {} fail,response code: {}", _name, rsp.status_code);
					return false;
				}
				std::cout << rsp.status_code << std::endl;
				std::cout << rsp.text << std::endl;
			}
			catch (std::exception &e)
			{
				LOG_ERROR("Creating ES index {} fail: {}", _name, e.what());
				return false;
			}

			return true;
		}
		std::string serialzie()
		{
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
	public:
		EsInsert(std::shared_ptr<elasticlient::Client> &client,
				 const std::string &name,
				 const std::string &type = "_doc") : _name(name), _type(type), _client(client) {}
		template <typename T>
		EsInsert &append(const std::string &key, const T &val)
		{
			_item[key] = val;
			return *this;
		}
		bool insert(const std::string id = "")
		{
			std::string body = JsonUtil::serialize(_item);
			if (body.empty())
			{
				LOG_ERROR("Index serialize fail!");
				return false;
			}
			// LOG_DEBUG("{}", body);
			// 2. 发起搜索请求
			try
			{
				auto rsp = _client->index(_name, _type, id, body);
				if (rsp.status_code < 200 || rsp.status_code >= 300)
				{
					LOG_ERROR("Add data {} fail,Error rsp_code: {}", body, rsp.status_code);
					return false;
				}
			}
			catch (std::exception &e)
			{
				LOG_ERROR("Add data {} fail: {}", body, e.what());
				return false;
			}
			return true;
		}

	private:
		std::string _name;
		std::string _type;
		Json::Value _item;
		std::shared_ptr<elasticlient::Client> _client;
	};

	class EsRemove
	{
	public:
		EsRemove(std::shared_ptr<elasticlient::Client> &client,
				 const std::string &name,
				 const std::string &type = "_doc") : _name(name), _type(type), _client(client) {}
		bool remove(const std::string &id)
		{
			try
			{
				auto rsp = _client->remove(_name, _type, id);
				if (rsp.status_code < 200 || rsp.status_code >= 300)
				{
					LOG_ERROR("Delete {} fail,rsp code error: {}", id, rsp.status_code);
					return false;
				}
			}
			catch (std::exception &e)
			{
				LOG_ERROR("delete {} fail: {}", id, e.what());
				return false;
			}
			return true;
		}

	private:
		std::string _name;
		std::string _type;
		std::shared_ptr<elasticlient::Client> _client;
	};

	class EsSearch
	{
	public:
		EsSearch(std::shared_ptr<elasticlient::Client> &client,
				 const std::string &name,
				 const std::string &type = "_doc") : _name(name), _type(type), _client(client) {}
		EsSearch &append_must_not_terms(const std::string &key, const std::vector<std::string> &vals)
		{
			Json::Value fields;
			for (const auto &val : vals)
			{
				fields[key].append(val);
			}
			Json::Value terms;
			terms["terms"] = fields;
			_must_not.append(terms);
			return *this;
		}
		EsSearch &append_should_match(const std::string &key, const std::string &val)
		{
			Json::Value field;
			field[key] = val;
			Json::Value match;
			match["match"] = field;
			_should.append(match);
			return *this;
		}
		EsSearch &append_must_term(const std::string &key, const std::string &val)
		{
			Json::Value field;
			field[key] = val;
			Json::Value term;
			term["term"] = field;
			_must.append(term);
			return *this;
		}
		EsSearch &append_must_match(const std::string &key, const std::string &val)
		{
			Json::Value field;
			field[key] = val;
			Json::Value match;
			match["match"] = field;
			_must.append(match);
			return *this;
		}
		Json::Value search()
		{
			Json::Value cond;
			if (_must_not.empty() == false)
				cond["must_not"] = _must_not;
			if (_should.empty() == false)
				cond["should"] = _should;
			if (_must.empty() == false)
				cond["must"] = _must;
			Json::Value query;
			query["bool"] = cond;
			Json::Value root;
			root["query"] = query;

			std::string body = JsonUtil::serialize(root);
			if (body.empty())
			{
				LOG_ERROR("Index serialize fail！");
				return Json::Value();
			}
			// LOG_DEBUG("{}", body);
			// 2. 发起搜索请求
			cpr::Response rsp;
			try
			{
				rsp = _client->search(_name, _type, body);
				if (rsp.status_code < 200 || rsp.status_code >= 300)
				{
					LOG_ERROR("search {} fail,error code: {}", body, rsp.status_code);
					return Json::Value();
				}
			}
			catch (std::exception &e)
			{
				LOG_ERROR("search {} fail: {}", body, e.what());
				return Json::Value();
			}
			// 3. 需要对响应正文进行反序列化
			LOG_DEBUG("search resp: [{}]", rsp.text);
			Json::Value json_res = JsonUtil::deserialize(rsp.text);
			if (json_res.empty())
			{
				LOG_ERROR("search data {} deserialize fail", rsp.text);
				return Json::Value();
			}
			return json_res["hits"]["hits"];
		}

	private:
		std::string _name;
		std::string _type;
		Json::Value _must_not;
		Json::Value _should;
		Json::Value _must;
		std::shared_ptr<elasticlient::Client> _client;
	};

}