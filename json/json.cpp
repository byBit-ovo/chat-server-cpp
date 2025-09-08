#include <json/json.h>
#include <memory>
#include <sstream>
#include <iostream>
using std::cout ;
using std::endl;
class JsonUtil
{
    public:

        static std::string serialize(const Json::Value &json)
        {
            std::stringstream ss;
            Json::StreamWriterBuilder factory;
            std::unique_ptr<Json::StreamWriter> writer(factory.newStreamWriter());
            writer->write(json, &ss);
            return ss.str();
        }
        static Json::Value deserialize(const std::string &src)
        {
            Json::CharReaderBuilder factory;
            std::unique_ptr<Json::CharReader> reader(factory.newCharReader());
            Json::Value root;
            std::string err;
            if (reader->parse(src.c_str(), src.c_str() + src.size(), &root, &err) == false)
            {
            }
            return root;
        }
};
int main()
{
	Json::Value root;
	root["name"]="bybit";
	root["age"] = 45;
	float scores[10] = {1.3,4,5,90,34.5,86.7};
	for(auto f: scores)
	{
		root["Scores"].append(f);
	}
	std::string message = JsonUtil::serialize(root);
	cout << message << endl;
	Json::Value result = JsonUtil::deserialize(message);
	cout << result["name"].asString() << endl;
	cout << result["age"].asInt() << endl;
	for(int i=0; i < result["Scores"].size();++i)
	{
		cout << result["Scores"][i].asFloat() << endl;
	}
	return 0;
}