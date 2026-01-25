
#include <sw/redis++/redis.h>
#include <gflags/gflags.h>
#include <string>

namespace MY_IM
{
	class RedisFactory
	{
		public:
			using RedisPtr = std::shared_ptr<sw::redis::Redis>;
			static RedisPtr create(const std::string &ip,
				int port, int db,bool keep_alive)
			{
				sw::redis::ConnectionOptions opts;
				opts.host = ip;
				opts.port = port;
				opts.db = db;
				opts.keep_alive = keep_alive;
				RedisPtr client = std::make_shared<sw::redis::Redis>(opts);
				return client;
			}
	};

	class SessionClient
	{
		public:
            using ptr = std::shared_ptr<SessionClient>;
			SessionClient(std::shared_ptr<sw::redis::Redis> client):
			_redis_client(client)
			{}

			void append(const std::string &ssid, const std::string &uid) {
                _redis_client->set(ssid, uid);
            }
			void append(const std::string &ssid, const std::string &uid, const std::string &device_type) {
                _redis_client->set(ssid, pack(uid, device_type));
            }
            void remove(const std::string &ssid) {
                _redis_client->del(ssid);
            }
            sw::redis::OptionalString uid(const std::string &ssid) {
                auto res = _redis_client->get(ssid);
                if (!res) {
                    return sw::redis::OptionalString();
                }
                return unpack_uid(*res);
            }
			sw::redis::OptionalString device_type(const std::string &ssid) {
                auto res = _redis_client->get(ssid);
                if (!res) {
                    return sw::redis::OptionalString();
                }
                return unpack_device_type(*res);
            }

		private:
			static std::string pack(const std::string &uid, const std::string &device_type) {
                return uid + "|" + device_type;
            }
			static sw::redis::OptionalString unpack_uid(const std::string &value) {
                auto pos = value.find('|');
                if (pos == std::string::npos) {
                    return sw::redis::OptionalString(value);
                }
                return sw::redis::OptionalString(value.substr(0, pos));
            }
			static sw::redis::OptionalString unpack_device_type(const std::string &value) {
                auto pos = value.find('|');
                if (pos == std::string::npos) {
                    return sw::redis::OptionalString();
                }
                return sw::redis::OptionalString(value.substr(pos + 1));
            }
			std::shared_ptr<sw::redis::Redis> _redis_client;
	};

	class StatusClient {
        public:
            using ptr = std::shared_ptr<StatusClient>;
            StatusClient(const std::shared_ptr<sw::redis::Redis> &redis_client):
                _redis_client(redis_client){}
            void append(const std::string &uid) {
                _redis_client->set(uid, "");
            }
            void remove(const std::string &uid) {
                _redis_client->del(uid);
            }
            bool exists(const std::string &uid) {
                auto res = _redis_client->get(uid);
                if (res) return true;
                return false;
            }
        private:
            std::shared_ptr<sw::redis::Redis> _redis_client;
    };

	class CodesClient {
        public:
            using ptr = std::shared_ptr<CodesClient>;
            CodesClient(const std::shared_ptr<sw::redis::Redis> &redis_client):
                _redis_client(redis_client){}
            void append(const std::string &cid, const std::string &code, 
                const std::chrono::milliseconds &t = std::chrono::milliseconds(300000)) {
                _redis_client->set(cid, code, t);
            }
            void remove(const std::string &cid) {
                _redis_client->del(cid);
            }
            sw::redis::OptionalString code(const std::string &cid)  {
                return _redis_client->get(cid);
            }
        private:
            std::shared_ptr<sw::redis::Redis> _redis_client;
    };

	class SequenceClient {
        public:
            using ptr = std::shared_ptr<SequenceClient>;
            SequenceClient(const std::shared_ptr<sw::redis::Redis> &redis_client):
                _redis_client(redis_client){}
            long long next(const std::string &key) {
                return _redis_client->incr(key);
            }
        private:
            std::shared_ptr<sw::redis::Redis> _redis_client;
    };

}
