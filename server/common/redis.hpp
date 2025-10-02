#include <sw/redis++/redis.h>
#include <gflags/gflags.h>

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
            void remove(const std::string &ssid) {
                _redis_client->del(ssid);
            }
            sw::redis::OptionalString uid(const std::string &ssid) {
                return _redis_client->get(ssid);
            }

		private:
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

}
