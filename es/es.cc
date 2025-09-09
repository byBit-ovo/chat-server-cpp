// POST /user/_doc
// { 
//  "settings" : {
//  "analysis" : {
//  	"analyzer" : {
//  		"ik" : {
//  			"tokenizer" : "ik_max_word"
//  		} 
//  	} 
//  } 
//  },
//  "mappings" : {
		//  "dynamic" : true,
		//  "properties" : {
		//  "nickname" : {
			//  "type" : "text",
			//  "analyzer" : "ik_max_word"
	//  },
//  "user_id" : {
//  "type" : "keyword",
//  "analyzer" : "standard"
//  },
//  "phone" : {
//  "type" : "keyword",
//  "analyzer" : "standard"
//  },
//  "description" : {
//  "type" : "text",
//  "enabled" : false
//  },
//  "avatar_id" : {
//  "type" : "keyword"
//  "enabled" : false
//  } 
//  } 
//  }
// }

// POST /user/_doc/_bulk
// {"index":{"_id":"1"}}
// {"user_id" : "USER4b862aaa-2df8654a-7eb4bb65-e3507f66","nickname" : "昵称 1","phone" : "手机号 1","description" : "签名 1","avatar_id" : "头像 1"}
// {"index":{"_id":"2"}}
// {"user_id" : "USER14eeeaa5-442771b9-0262e455-e4663d1d","nickname" : "昵称 2","phone" : "手机号 2","description" : "签名 2","avatar_id" : "头像 2"}
// {"index":{"_id":"3"}}
// {"user_id" : "USER484a6734-03a124f0-996c169d-d05c1869","nickname" : "昵称 3","phone" : "手机号 3","description" : "签名 3","avatar_id" : "头像 3"}
// {"index":{"_id":"4"}}
// {"user_id" : "USER186ade83-4460d4a6-8c08068f-83127b5d","nickname" : "昵称 4","phone" : "手机号 4","description" : "签名 4","avatar_id" : "头像 4"}
// {"index":{"_id":"5"}}
// {"user_id" : "USER6f19d074-c33891cf-23bf5a83-57189a19","nickname" : "昵称 5","phone" : "手机号 5","description" : "签名 5","avatar_id" : "头像 5"}
// {"index":{"_id":"6"}}
// {"user_id" : "USER97605c64-9833ebb7-d0455353-35a59195","nickname" : "昵称 6","phone" : "手机号 6","description" : "签名 6","avatar_id" : "头像 6"}


// GET /test_user/_doc/_search
// {
//   "query":
//   {
//     "match_all":{}
//   }
// }
#include <gflags/gflags.h>
#include "../common/esclient.hpp"
#include <memory>
DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");


int main(int argc, char *argv[])
{
	google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
	std::vector<std::string> hosts ={	
		"http://127.0.0.1:9200/"
	};
	auto client = std::make_shared<elasticlient::Client>(hosts);
	EsIndex index(client,"test_user","_doc");
	index.Append("key1").Append("key2","keyword").Append("key3","keyword");
	if(index.create() == false)
	{
		LOG_ERROR("Creating index fail");
	}
	
	return 0;
}