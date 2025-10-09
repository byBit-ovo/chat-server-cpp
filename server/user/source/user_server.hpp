
#include <brpc/server.h>

#include "etcd.hpp"
#include "channel.hpp"
#include "utils.hpp"
#include "redis.hpp"
#include "mysql.hpp"
#include "search.hpp"
#include "user.pb.h"
#include "file.pb.h"	// call file micro server for avatar
#include "base.pb.h"
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
			}
			virtual ~UserServiceImplement() {}
			bool nickname_check(const std::string &nickname) {
				return nickname.size() < 22;
			}
        bool password_check(const std::string &password) {
            if (password.size() < 6 || password.size() > 15) {
                LOG_ERROR("密码长度不合法：{}-{}", password, password.size());
                return false;
            }
            for (int i = 0; i < password.size(); i++) {
                if (!((password[i] >= 'a' && password[i] <= 'z') ||
                    (password[i] >= 'A' && password[i] <= 'Z') ||
                    (password[i] >= '0' && password[i] <= '9') ||
                    password[i] == '_' || password[i] == '-')) {
                    	LOG_ERROR("密码字符不合法：{}", password);
                    	return false;
                	}
				}
				return true;
			}
			virtual void UserRegister
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::UserRegisterReq *request,
				::MY_IM::UserRegisterRsp *response,
				::google::protobuf::Closure *done
			)
			{
				// sync call
				brpc::ClosureGuard guard(done);
				//定义一个错误处理函数，当出错的时候被调用
				auto err_response = [this, response](const std::string &rid, 
					const std::string &errmsg) -> void {
					response->set_request_id(rid);
					response->set_success(false);
					response->set_errmsg(errmsg);
					return;
				};
				//1. 从请求中取出昵称和密码
				std::string nickname = request->nickname();
				std::string password = request->password();
				//2. 检查昵称是否合法（只能包含字母，数字，连字符-，下划线_，长度限制 3~15 之间）
				bool ret = nickname_check(nickname);
				if (ret == false) {
					LOG_ERROR("{} - 用户名长度不合法！", request->request_id());
					return err_response(request->request_id(), "用户名长度不合法！");
				}
				//3. 检查密码是否合法（只能包含字母，数字，长度限制 6~15 之间）
				ret = password_check(password);
				if (ret == false) {
					LOG_ERROR("{} - 密码格式不合法！", request->request_id());
					return err_response(request->request_id(), "密码格式不合法！");
				}
				//4. 根据昵称在数据库进行判断是否昵称已存在
				auto user = _user_table->select_by_nickname(nickname);
				if (user) {
					LOG_ERROR("{} - 用户名被占用- {}！", request->request_id(), nickname);
					return err_response(request->request_id(), "用户名被占用!");
				}
				//5. 向数据库新增数据
				std::string uid = Uuid();
				user = std::make_shared<User>(uid, nickname, password);
				ret = _user_table->insert(user);
				if (ret == false) {
					LOG_ERROR("{} - Mysql数据库新增数据失败！", request->request_id());
					return err_response(request->request_id(), "Mysql数据库新增数据失败!");
				}
				//6. 向 ES 服务器中新增用户信息
				ret = _es_user->appendData(uid, "", nickname, "", "");
				if (ret == false) {
					LOG_ERROR("{} - ES搜索引擎新增数据失败！", request->request_id());
					return err_response(request->request_id(), "ES搜索引擎新增数据失败！");
				}
				//7. 组织响应，进行成功与否的响应即可。
				response->set_request_id(request->request_id());
				response->set_success(true);
			}
			virtual void UserLogin
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::UserLoginReq *request,
				::MY_IM::UserLoginRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);
				auto err_response = [this, response](const std::string &rid, 
					const std::string &errmsg) -> void {
					response->set_request_id(rid);
					response->set_success(false);
					response->set_errmsg(errmsg);
					return;
				};
				//1. 从请求中取出昵称和密码
				std::string nickname = request->nickname();
				std::string password = request->password();
				//2. 通过昵称获取用户信息，进行密码是否一致的判断
				auto user = _user_table->select_by_nickname(nickname);
				if (!user || password != user->password()) {
					LOG_ERROR("{} - 用户名或密码错误 - {}-{}！", request->request_id(), nickname, password);
					return err_response(request->request_id(), "用户名或密码错误!");
				}
				//3. 根据 redis 中的登录标记信息是否存在判断用户是否已经登录。
				bool ret = _status_client->exists(user->user_id());
				if (ret == true) {
					LOG_ERROR("{} - 用户已在其他地方登录 - {}！", request->request_id(), nickname);
					return err_response(request->request_id(), "用户已在其他地方登录!");
				}
				//4. 构造会话 ID，生成会话键值对，向 redis 中添加会话信息以及登录标记信息
				std::string ssid = Uuid();
				_session_client->append(ssid, user->user_id());
				//5. 添加用户登录信息
				_status_client->append(user->user_id());
				//5. 组织响应，返回生成的会话 ID
				response->set_request_id(request->request_id());
				response->set_login_session_id(ssid);
				response->set_success(true);
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
			// operators after login
			virtual void GetUserInfo
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::GetUserInfoReq *request,
				::MY_IM::GetUserInfoRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);
				auto err_response = [this, response](const std::string &rid, 
					const std::string &errmsg) -> void {
					response->set_request_id(rid);
					response->set_success(false);
					response->set_errmsg(errmsg);
					return;
            	};
				// 1. 从请求中取出用户 ID
				std::string uid = request->user_id();
				// 2. 通过用户 ID，从数据库中查询用户信息
				auto user = _user_table->select_by_id(uid);
				if (!user) {
					LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
					return err_response(request->request_id(), "未找到用户信息!");
				}
				// 3. 根据用户信息中的头像 ID，从文件服务器获取头像文件数据，组织完整用户信息
				UserInfo *user_info = response->mutable_user_info();
				user_info->set_user_id(user->user_id());
				user_info->set_nickname(user->nickname());
				user_info->set_description(user->description());
				user_info->set_phone(user->phone());
				
				if (!user->avatar_id().empty()) {
					//从信道管理对象中，获取到连接了文件管理子服务的channel
					auto channel = _channel_manager->GetChannel(_file_service_name);
					if (!channel) {
						LOG_ERROR("{} - 未找到文件管理子服务节点 - {} - {}！", 
							request->request_id(), _file_service_name, uid);
						return err_response(request->request_id(), "未找到文件管理子服务节点!");
					}
					//进行文件子服务的rpc请求，进行头像文件下载
					MY_IM::FileService_Stub stub(channel.get());
					MY_IM::GetSingleFileReq req;
					MY_IM::GetSingleFileRsp rsp;
					req.set_request_id(request->request_id());
					req.set_file_id(user->avatar_id());
					brpc::Controller cntl;
					stub.GetSingleFile(&cntl, &req, &rsp, nullptr);
					if (cntl.Failed() == true || rsp.success() == false) {
						LOG_ERROR("{} - 文件子服务调用失败：{}！", request->request_id(), cntl.ErrorText());
						return err_response(request->request_id(), "文件子服务调用失败!");
					}
					user_info->set_avatar(rsp.file_data().file_content());
				}
				// 4. 组织响应，返回用户信息
				response->set_request_id(request->request_id());
				response->set_success(true);

			}
			virtual void GetMultiUserInfo
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::GetMultiUserInfoReq *request,
				::MY_IM::GetMultiUserInfoRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);
				//1. 定义错误回调
				auto err_response = [this, response](const std::string &rid, 
					const std::string &errmsg) -> void {
					response->set_request_id(rid);
					response->set_success(false);
					response->set_errmsg(errmsg);
					return;
				};
				//2. 从请求中取出用户ID --- 列表
				std::vector<std::string> uid_lists;
				for (int i = 0; i < request->users_id_size(); i++) {
					uid_lists.push_back(request->users_id(i));
				}
				//3. 从数据库进行批量用户信息查询
				auto users = _user_table->select_multi_users(uid_lists);
				if (users.size() != request->users_id_size()) {
					LOG_ERROR("{} - 从数据库查找的用户信息数量不一致 {}-{}！", 
						request->request_id(), request->users_id_size(), users.size());
					return err_response(request->request_id(), "从数据库查找的用户信息数量不一致!");
				}
				//4. 批量从文件管理子服务进行文件下载
				auto channel = _channel_manager->GetChannel(_file_service_name);
				if (!channel) {
					LOG_ERROR("{} - 未找到文件管理子服务节点 - {}！", request->request_id(), _file_service_name);
					return err_response(request->request_id(), "未找到文件管理子服务节点!");
				}
				MY_IM::FileService_Stub stub(channel.get());
				MY_IM::GetMultiFileReq req;
				MY_IM::GetMultiFileRsp rsp;
				req.set_request_id(request->request_id());
				for (auto &user : users) {
					if (user.avatar_id().empty()) continue;
					req.add_file_id_list(user.avatar_id());
				}
				brpc::Controller cntl;
				stub.GetMultiFile(&cntl, &req, &rsp, nullptr);
				if (cntl.Failed() == true || rsp.success() == false) {
					LOG_ERROR("{} - 文件子服务调用失败：{} - {}！", request->request_id(), 
						_file_service_name, cntl.ErrorText());
					return err_response(request->request_id(), "文件子服务调用失败!");
				}
				//5. 组织响应（）
				for (auto &user : users) {
					auto user_map = response->mutable_users_info();//本次请求要响应的用户信息map
					auto file_map = rsp.mutable_file_data(); //这是批量文件请求响应中的map 
					UserInfo user_info;
					user_info.set_user_id(user.user_id());
					user_info.set_nickname(user.nickname());
					user_info.set_description(user.description());
					user_info.set_phone(user.phone());
					user_info.set_avatar((*file_map)[user.avatar_id()].file_content());
					(*user_map)[user_info.user_id()] = user_info;
				}
				response->set_request_id(request->request_id());
				response->set_success(true);

			}
			virtual void SetUserAvatar
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::SetUserAvatarReq *request,
				::MY_IM::SetUserAvatarRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);
				auto err_response = [this, response](const std::string &rid, 
					const std::string &errmsg) -> void {
					response->set_request_id(rid);
					response->set_success(false);
					response->set_errmsg(errmsg);
					return;
            	};
				// 1. 从请求中取出用户 ID 与头像数据
				std::string uid = request->user_id();
				// 2. 从数据库通过用户 ID 进行用户信息查询，判断用户是否存在
				auto user = _user_table->select_by_id(uid);
				if (!user) {
					LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
					return err_response(request->request_id(), "未找到用户信息!");
				}
				// 3. 上传头像文件到文件子服务，
				auto channel = _channel_manager->GetChannel(_file_service_name);
				if (!channel) {
					LOG_ERROR("{} - 未找到文件管理子服务节点 - {}！", request->request_id(), _file_service_name);
					return err_response(request->request_id(), "未找到文件管理子服务节点!");
				}
				MY_IM::FileService_Stub stub(channel.get());
				MY_IM::PutSingleFileReq req;
				MY_IM::PutSingleFileRsp rsp;
				req.set_request_id(request->request_id());
				req.mutable_file_data()->set_file_name("");
				req.mutable_file_data()->set_file_size(request->avatar().size());
				req.mutable_file_data()->set_file_content(request->avatar());
				brpc::Controller cntl;
				stub.PutSingleFile(&cntl, &req, &rsp, nullptr);
				if (cntl.Failed() == true || rsp.success() == false) {
					LOG_ERROR("{} - 文件子服务调用失败：{}！", request->request_id(), cntl.ErrorText());
					return err_response(request->request_id(), "文件子服务调用失败!");
				}
				std::string avatar_id = rsp.file_info().file_id();
				// 4. 将返回的头像文件 ID 更新到数据库中
				user->avatar_id(avatar_id);
				bool ret = _user_table->update(user);
				if (ret == false) {
					LOG_ERROR("{} - 更新数据库用户头像ID失败 ：{}！", request->request_id(), avatar_id);
					return err_response(request->request_id(), "更新数据库用户头像ID失败!");
				}
				// 5. 更新 ES 服务器中用户信息
				ret = _es_user->appendData(user->user_id(), user->phone(),
					user->nickname(), user->description(), user->avatar_id());
				if (ret == false) {
					LOG_ERROR("{} - 更新搜索引擎用户头像ID失败 ：{}！", request->request_id(), avatar_id);
					return err_response(request->request_id(), "更新搜索引擎用户头像ID失败!");
				}
				// 6. 组织响应，返回更新成功与否
				response->set_request_id(request->request_id());
				response->set_success(true);

			}
			virtual void SetUserNickname
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::SetUserNicknameReq *request,
				::MY_IM::SetUserNicknameRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);
				auto err_response = [this, response](const std::string &rid, 
					const std::string &errmsg) -> void {
					response->set_request_id(rid);
					response->set_success(false);
					response->set_errmsg(errmsg);
					return;
				};
				// 1. 从请求中取出用户 ID 与新的昵称
				std::string uid = request->user_id();
				std::string new_nickname = request->nickname();
				// 2. 判断昵称格式是否正确
				bool ret = nickname_check(new_nickname);
				if (ret == false) {
					LOG_ERROR("{} - 用户名长度不合法！", request->request_id());
					return err_response(request->request_id(), "用户名长度不合法！");
				}
				// 3. 从数据库通过用户 ID 进行用户信息查询，判断用户是否存在
				auto user = _user_table->select_by_id(uid);
				if (!user) {
					LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
					return err_response(request->request_id(), "未找到用户信息!");
				}
				// 4. 将新的昵称更新到数据库中
				user->nickname(new_nickname);
				ret = _user_table->update(user);
				if (ret == false) {
					LOG_ERROR("{} - 更新数据库用户昵称失败 ：{}！", request->request_id(), new_nickname);
					return err_response(request->request_id(), "更新数据库用户昵称失败!");
				}
				// 5. 更新 ES 服务器中用户信息
				ret = _es_user->appendData(user->user_id(), user->phone(),
					user->nickname(), user->description(), user->avatar_id());
				if (ret == false) {
					LOG_ERROR("{} - 更新搜索引擎用户昵称失败 ：{}！", request->request_id(), new_nickname);
					return err_response(request->request_id(), "更新搜索引擎用户昵称失败!");
				}
				// 6. 组织响应，返回更新成功与否
				response->set_request_id(request->request_id());
				response->set_success(true);

			}
			virtual void SetUserDescription
			(	google::protobuf::RpcController *controller,
				const ::MY_IM::SetUserDescriptionReq *request,
				::MY_IM::SetUserDescriptionRsp *response,
				::google::protobuf::Closure *done
			)
			{
				brpc::ClosureGuard guard(done);
				auto err_response = [this, response](const std::string &rid, 
					const std::string &errmsg) -> void {
					response->set_request_id(rid);
					response->set_success(false);
					response->set_errmsg(errmsg);
					return;
				};
				// 1. 从请求中取出用户 ID 与新的昵称
				std::string uid = request->user_id();
				std::string new_description = request->description();
				// 3. 从数据库通过用户 ID 进行用户信息查询，判断用户是否存在
				auto user = _user_table->select_by_id(uid);
				if (!user) {
					LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
					return err_response(request->request_id(), "未找到用户信息!");
				}
				// 4. 将新的昵称更新到数据库中
				user->description(new_description);
				bool ret = _user_table->update(user);
				if (ret == false) {
					LOG_ERROR("{} - 更新数据库用户签名失败 ：{}！", request->request_id(), new_description);
					return err_response(request->request_id(), "更新数据库用户签名失败!");
				}
				// 5. 更新 ES 服务器中用户信息
				ret = _es_user->appendData(user->user_id(), user->phone(),
					user->nickname(), user->description(), user->avatar_id());
				if (ret == false) {
					LOG_ERROR("{} - 更新搜索引擎用户签名失败 ：{}！", request->request_id(), new_description);
					return err_response(request->request_id(), "更新搜索引擎用户签名失败!");
				}
				// 6. 组织响应，返回更新成功与否
				response->set_request_id(request->request_id());
				response->set_success(true);
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
				const std::shared_ptr<brpc::Server> &rpc_server,
				const Registerant::ptr &reg_client,
				const Discoverer::ptr &discover_client,
				UserServiceImplement *user_service
			):
				_rpc_server(rpc_server),
				_register_client(reg_client),
				_discover_client(discover_client),
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
				return *this;

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
				return *this;
			}

			ref Construct_Redis_Client(
				const std::string &ip,
				int port,
				int db,
				bool keep_alive
			)
			{
				_redis_client = RedisFactory::create(ip, port, db, keep_alive);
				return *this;
			}

			ref Construct_Discover_Client
			(	const std::string &host, const std::string &base_dir,
				const std::string &file_service_name
			)
			{
				_file_service_name = file_service_name;
				_channel_manager = std::make_shared<ServiceManager>();
				_channel_manager->FollowOn(file_service_name);
				auto put_call = std::bind(&ServiceManager::OnlineCall,_channel_manager.get(),
				std::placeholders::_1,std::placeholders::_2);
				auto del_call = std::bind(&ServiceManager::OfflineCall,_channel_manager.get(),
				std::placeholders::_1,std::placeholders::_2);
				_discover_client = std::make_shared<Discoverer>(host,base_dir,put_call,del_call);

				return *this;
			}

			// 2.
			ref Construct_Rpc_Server(int port, int ttl, int thread_nums)
			{
				if (!_es_client) {
                	LOG_ERROR("You should initialize ES module first!");
                	abort();
				}
				if (!_mysql_client) {
                	LOG_ERROR("You should initialize MySQL module first!");
					abort();
				}
				if (!_redis_client) {
                	LOG_ERROR("You should initialize Redis module first!");
					abort();
				}
				if (!_channel_manager) {
                	LOG_ERROR("You should initialize ChannelManager module first!");
					abort();
				}

				_rpc_server = std::make_shared<brpc::Server>();
				_user_service = new UserServiceImplement(_es_client,_mysql_client,_redis_client,
					_channel_manager,_file_service_name);
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
					LOG_WARNING("You should implement service and listener before registering to etcd");
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
					_rpc_server.get() == nullptr ||
					_discover_client.get() == nullptr)
				{
					LOG_WARNING("You should implement dicoverer、register、rpc_server、user_service before build UserServer!");
					abort();
				}
				return std::make_shared<UserServer>(_rpc_server, _register_client, _discover_client,_user_service);
			}

		private:
			// these five members are for UserServiceImplement
			std::shared_ptr<elasticlient::Client> _es_client;
			std::shared_ptr<odb::core::database> _mysql_client;
			std::shared_ptr<sw::redis::Redis> _redis_client;
			ServiceManager::ptr _channel_manager;
			std::string _file_service_name;
			// these four members are for UserServer
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			UserServiceImplement *_user_service;
			Discoverer::ptr _discover_client;
	};
}