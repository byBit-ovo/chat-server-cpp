#include <json/json.h>
#include <memory>
#include <sstream>
#include <iostream>
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
				return Json::Value();
            }
            return root;
        }
};
