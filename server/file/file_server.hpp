
#include <brpc/server.h>
#include <butil/logging.h>
#include <gflags/gflags.h>
#include "../common/etcd.hpp"
#include "file.pb.h"
#include "base.pb.h"

namespace MY_IM
{
	class FileServiceImplement : public FileService
	{
	public:
		virtual void GetSingleFile(
			google::protobuf::RpcController *controller,
			const GetSingleFileReq *request,
			GetSingleFileRsp *response,
			::google::protobuf::Closure *done) override
		{
		}
		virtual void GetMultiFile(
			google::protobuf::RpcController *controller,
			const GetMultiFileReq *request,
			GetMultiFileRsp *response,
			::google::protobuf::Closure *done) override
		{
		}
		virtual void PutSingleFile(
			google::protobuf::RpcController *controller,
			const PutSingleFileReq *request,
			PutSingleFileRsp *response,
			::google::protobuf::Closure *done) override
		{
		}
		virtual void PutMultiFile(
			google::protobuf::RpcController *controller,
			const PutMultiFileReq *request,
			PutMultiFileRsp *response,
			::google::protobuf::Closure *done) override
		{

		}

	private:
		
	};

	class FileServer
	{
		public:
			using ptr = std::shared_ptr<SpeechServer>;
			FileServer(
				std::shared_ptr<brpc::Server> rpc_server,
				Registerant::ptr register_client,
				FileServiceImplement* file_service):
				_rpc_server(rpc_server),
				_register_client(register_client),
				_file_service(file_service)
			{}
			void Start()
			{
				_rpc_server->RunUntilAskedToQuit();
			}
		private:
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			FileServiceImplement* _file_service;
			
	};

	class FileServerBuilder
	{
		public:
			using ref = FileServerBuilder&;

			// 1.
			ref Construct_File_Service()
			{
				_file_service = new FileServiceImplement();
				return *this;
			}
			
			// 2.
			ref Construct_Rpc_Server(int port,int ttl,int thread_nums)
			{
				if(_file_service == nullptr){
					LOG_WARNING("You should implement service and register before construct rpc server");
					return *this;
				}

				_rpc_server = std::make_shared<brpc::Server>();
				//注意这里，_speech_service的 生命周期/所有权 要交给_rpc_server来管理，所以该server拥有该服务
				if(-1 == _rpc_server->AddService(_file_service,brpc::ServiceOwnership::SERVER_OWNS_SERVICE))
				{
					LOG_ERROR("Add service error");
					exit(1);
				}
				brpc::ServerOptions op;
				op.idle_timeout_sec = ttl;
				op.num_threads = thread_nums;
				if(_rpc_server->Start(port,&op)  == -1){
					LOG_ERROR("Server start error");
					exit(1);
				} 
				return *this;
			}
			// 3.
			ref Construct_Register(
				const std::string &etcd_host,
				const std::string &service_name,
				const std::string &service_addr
			)
			{
				if(_file_service == nullptr || _rpc_server.get() == nullptr){
					LOG_WARNING("You should implement service and listen before registering to etcd");
					exit(1);
				}
				_register_client = std::make_shared<Registerant>(etcd_host);
				bool ret = _register_client->Register(service_name,service_addr);
				if(ret == false)
				{
					LOG_ERROR("Register service({}) fail,exit", service_name);
					exit(1);
				}
				LOG_INFO("Registered a service-->{}:{}",service_name,service_addr);
				return *this;
			}

			FileServer::ptr Build()
			{
				if(_register_client.get() == nullptr || 
					_file_service == nullptr || 
					_rpc_server.get()==nullptr)
				{
					LOG_WARNING("You should implement service&register*rpc_server before build SpeechServer!");
					abort();
				}
				return std::make_shared<FileServer>(_rpc_server,_register_client,_file_service);
			}

		private:
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			// SpeechServiceImplement::ptr _speech_service;
			FileServerImplement* _file_service;
	};
}
