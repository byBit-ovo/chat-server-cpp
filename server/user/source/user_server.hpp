#include "etcd.hpp"
#include "../../common/channel.hpp"
#include "redis.hpp"
#include "mysql.hpp"
#include "search.hpp"
#include "user.pb.h"
#include "file.pb.h"	// call file micro server for avatar
#include "base.pb.h"
#include <brpc/server.h>
#include <butil/logging.h>
#include <gflags/gflags.h>

namespace MY_IM
{
	class UserServiceImplement : public UserService
	{
		public:
			using ptr = std::shared_ptr<UserServiceImplement>;
			UserServiceImplement
			(
				const std::shared_ptr<elasticlient::Client> &es_client,
				const std::shared_ptr<odb::core::database> &mysql_client,
				const std::shared_ptr<sw::redis::Redis> &redis_client,
				const ServiceManager::ptr &channel_manager,
				const std::string &file_service_name
			):
				_es_user(std::make_shared<ESUser>(es_client)),
				_user_table(std::make_shared<UserTable>(mysql_client)),
				_session_client(std::make_shared<SessionClient>(redis_client)),
				_status_client(std::make_shared<StatusClient>(redis_client)),
				_codes_client(std::make_shared<CodesClient>(redis_client)),
				_file_service_name(file_service_name),
				_channel_manager(channel_manager)
			{
				Registerant::ptr _register_client;
				std::string _file_service_name;
			}
			virtual ~UserServiceImplement() {}
			virtual void UserRegister
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::UserRegisterReq *request,
				::MY_IM::UserRegisterRsp *response,
				::google::protobuf::Closure *done
			)
			{
				// sync call
				brpc::ClosureGuard guard(done);

			}
			virtual void UserLogin
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::UserLoginReq *request,
				::MY_IM::UserLoginRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}

			virtual void GetPhoneVerifyCode
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::PhoneVerifyCodeReq *request,
				::MY_IM::PhoneVerifyCodeRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}
			virtual void PhoneRegister
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::PhoneRegisterReq *request,
				::MY_IM::PhoneRegisterRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}
			virtual void PhoneLogin
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::PhoneLoginReq *request,
				::MY_IM::PhoneLoginRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}
			virtual void GetUserInfo
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::GetUserInfoReq *request,
				::MY_IM::GetUserInfoRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}
			virtual void GetMultiUserInfo
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::GetMultiUserInfoReq *request,
				::MY_IM::GetMultiUserInfoRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}
			virtual void SetUserAvatar
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::SetUserAvatarReq *request,
				::MY_IM::SetUserAvatarRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}
			virtual void SetUserNickname
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::SetUserNicknameReq *request,
				::MY_IM::SetUserNicknameRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}
			virtual void SetUserDescription
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::SetUserDescriptionReq *request,
				::MY_IM::SetUserDescriptionRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}
			virtual void SetUserPhoneNumber
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::SetUserPhoneNumberReq *request,
				::MY_IM::SetUserPhoneNumberRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);

			}

		private:
			ESUser::ptr _es_user;
			UserTable::ptr _user_table;
			SessionClient::ptr _session_client;
			StatusClient::ptr _status_client;
			CodesClient::ptr _codes_client;
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			std::string _file_service_name;
			ServiceManager::ptr _channel_manager;

	};

	class UserServer
	{
		public:
			using ptr = std::shared_ptr<UserServer>;
			UserServer(
				std::shared_ptr<brpc::Server> rpc_server,
				Registerant::ptr register_client,
				UserServiceImplement *user_service,
				const std::string &host, const std::string &base_dir,
				const Discoverer::call_back_t &put_call, 
				const Discoverer::call_back_t &del_call
			):
				_rpc_server(rpc_server),
				_register_client(register_client),
				_user_service(user_service)
			{
			}
			void Start()
			{
				_rpc_server->RunUntilAskedToQuit();
			}

		private:
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			Discoverer::ptr _discover_client;
			UserServiceImplement *_user_service;
	};

	class UserServerBuilder
	{
		public:
			using ref = UserServerBuilder &;

			// 1.
			ref Construct_Es_Client(const std::vector<std::string>& hosts)
			{
				// std::vector<std::string> hosts ={	
				// 	"http://127.0.0.1:9200/"
				// };
				// auto client = std::make_shared<elasticlient::Client>(hosts);
				_es_client = std::make_shared<elasticlient::Client>(hosts);

			}
			ref Construct_MySQL_Client(
				const std::string &user,
				const std::string &pswd,
				const std::string &host,
				const std::string &db,
				const std::string &cset,
				int port,
				int conn_pool_count
			)
			{
				// std::unique_ptr<odb::mysql::connection_pool_factory> cpf(
				// new odb::mysql::connection_pool_factory(FLAGS_max_pool, 0));
				// //2. 构造数据库操作对象
				// odb::mysql::database db(
				// FLAGS_user, FLAGS_pswd, FLAGS_db,
				// FLAGS_host, FLAGS_port, "", FLAGS_cset,
				// 0, std::move(cpf));
				_mysql_client = ODBFactory::create(user, pswd, host, db, cset, port, conn_pool_count);

			}

			ref Construct_Redis_Client(
				const std::string &ip,
				int port,
				int db,
				bool keep_alive
			)
			{
				_redis_client = RedisFactory::create(ip, port, db, keep_alive);
			}
			ref Construct_User_Service()
			{

				// const std::shared_ptr<elasticlient::Client> &es_client,
				// const std::shared_ptr<odb::core::database> &mysql_client,
				// const std::shared_ptr<sw::redis::Redis> &redis_client,
				// const ServiceManager::ptr &channel_manager,
				// const std::string &file_service_name

				return *this;
			}
			ref Construct_Discover_Client
			(	const std::string &host, const std::string &base_dir,
				const Discoverer::call_back_t &put_call, 
				const Discoverer::call_back_t &del_call
			)
			{
				_discover_client = std::make_shared<Discoverer>(host,base_dir,put_call,del_call);
				return *this;
			}

			// 2.
			ref Construct_Rpc_Server(int port, int ttl, int thread_nums)
			{
				if (_user_service == nullptr)
				{
					LOG_WARNING("You should implement service and register before construct rpc server");
					return *this;
				}

				_rpc_server = std::make_shared<brpc::Server>();
				// 注意这里，_user_service的 生命周期/所有权 要交给_rpc_server来管理，所以该server拥有该服务
				if (-1 == _rpc_server->AddService(_user_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE))
				{
					LOG_ERROR("Add service error");
					exit(1);
				}
				brpc::ServerOptions op;
				op.idle_timeout_sec = ttl;
				op.num_threads = thread_nums;
				if (_rpc_server->Start(port, &op) == -1)
				{
					LOG_ERROR("Server start error");
					exit(1);
				}
				return *this;
			}
			// 3.
			ref Construct_Register(
				const std::string &etcd_host,
				const std::string &service_name,
				const std::string &service_addr)
			{
				if (_user_service == nullptr || _rpc_server.get() == nullptr)
				{
					LOG_WARNING("You should implement service and listen before registering to etcd");
					exit(1);
				}
				_register_client = std::make_shared<Registerant>(etcd_host);
				bool ret = _register_client->Register(service_name, service_addr);
				if (ret == false)
				{
					LOG_ERROR("Register service({}) fail,exit", service_name);
					exit(1);
				}
				LOG_INFO("Registered a service-->{}:{}", service_name, service_addr);
				return *this;
			}

			UserServer::ptr Build()
			{
				if (_register_client.get() == nullptr ||
					_user_service == nullptr ||
					_rpc_server.get() == nullptr)
				{
					LOG_WARNING("You should implement service&register*rpc_server before build UserServer!");
					abort();
				}
				return std::make_shared<UserServer>(_rpc_server, _register_client, _user_service);
			}

		private:
			std::shared_ptr<elasticlient::Client> _es_client;
			std::shared_ptr<odb::core::database> _mysql_client;
			std::shared_ptr<sw::redis::Redis> _redis_client;
			ServiceManager::ptr _channel_manager;
			std::string _file_service_name;
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			UserServiceImplement *_user_service;
			Discoverer::ptr _discover_client;
	};
}