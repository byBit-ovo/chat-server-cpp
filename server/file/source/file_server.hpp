#include <brpc/server.h>
#include <butil/logging.h>
#include <gflags/gflags.h>
#include "etcd.hpp"
#include "file.pb.h"
#include "base.pb.h"
#include "utils.hpp"
#include "logger.hpp"
#include <sys/stat.h>
#include <sys/types.h>
namespace MY_IM
{
	class FileServiceImplement : public FileService
	{
	public:
		FileServiceImplement(const std::string &files_dir):
		_files_dir(files_dir)
		{
			umask(0);
			mkdir(files_dir.c_str(),0775);
			if(_files_dir.back() != '/')
			{
				_files_dir += '/';
			}
		}
		virtual void GetSingleFile(
			google::protobuf::RpcController *controller,
			const GetSingleFileReq *request,
			GetSingleFileRsp *response,
			::google::protobuf::Closure *done) override
		{
			brpc::ClosureGuard guard(done);
			std::string file_id = request->file_id();
			string file_content;
			response->set_request_id(request->request_id());
			bool ret = ReadFile(_files_dir + file_id,file_content);
			if(ret == false){
				response->set_success(false);
				response->set_errmsg("read file fail!");
				return;
			}
			response->set_success(true);
			response->mutable_file_data()->set_file_id(file_id);
			response->mutable_file_data()->set_file_content(file_content);
		}
		virtual void GetMultiFile(
			google::protobuf::RpcController *controller,
			const GetMultiFileReq *request,
			GetMultiFileRsp *response,
			::google::protobuf::Closure *done) override
		{
			brpc::ClosureGuard guard(done);

			response->set_request_id(request->request_id());

			for(int i = 0;i < request->file_id_list_size();++i)
			{
				string file_id = request->file_id_list(i);
				string file_content;
				bool ret = ReadFile(_files_dir + file_id,file_content);
				if(ret == false){
					response->clear_file_data();
					response->set_success(false);
					response->set_errmsg("read file fail!");
					return;
				}
			 	FileDownloadData file_data;
				file_data.set_file_id(file_id);
				file_data.set_file_content(file_content);
				response->mutable_file_data()->insert({file_id,file_data});
			}
			response->set_success(true);
		}
		virtual void PutSingleFile(
			google::protobuf::RpcController *controller,
			const PutSingleFileReq *request,
			PutSingleFileRsp *response,
			::google::protobuf::Closure *done) override
		{
			brpc::ClosureGuard guard(done);

			response->set_request_id(request->request_id());
			std::string uuid = Uuid();
			std::string file_path = _files_dir + uuid;
			// Because there may be mutiple clients uploading files with the same name,
			// the server use uuid instead of file nameto store 
			bool ret = WriteFile(file_path,request->file_data().file_content());
			if(ret == false){
				response->set_success(false);
				response->set_errmsg("write file fail!");
				return;
			}
			response->set_success(true);
			FileMessageInfo *msg_info = response->mutable_file_info();
			msg_info->set_file_id(uuid);
			msg_info->set_file_size(request->file_data().file_size());
			msg_info->set_file_name(request->file_data().file_name());
		}
		virtual void PutMultiFile(
			google::protobuf::RpcController *controller,
			const PutMultiFileReq *request,
			PutMultiFileRsp *response,
			::google::protobuf::Closure *done) override
		{
			brpc::ClosureGuard guard(done);
			response->set_request_id(request->request_id());
			for(int i = 0;i < request->file_data_size();++i)
			{
				string uuid = Uuid();
				string file_path = _files_dir + uuid;
				// Because there may be mutiple clients uploading files with the same name,
				// the server use uuid to store, instead of file name
				bool ret = WriteFile(file_path,request->file_data(i).file_content());
				if(ret == false){
					response->clear_file_info();
					response->set_success(false);
					response->set_errmsg("write file fail!");
					return;
				}
				FileMessageInfo* file_info = response->add_file_info();
				file_info->set_file_id(uuid);
				file_info->set_file_size(request->file_data(i).file_size());
				file_info->set_file_name(request->file_data(i).file_name());
			}
			response->set_success(true);
		}

	private:
		std::string _files_dir;
		
	};

	class FileServer
	{
		public:
			using ptr = std::shared_ptr<FileServer>;
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
			ref Construct_File_Service(const std::string &files_dir = "./asset/")
			{
				_file_service = new FileServiceImplement(files_dir);
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
					LOG_WARNING("You should implement service&register&rpc_server before build FileServer!");
					abort();
				}
				return std::make_shared<FileServer>(_rpc_server,_register_client,_file_service);
			}

		private:
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			// SpeechServiceImplement::ptr _speech_service;
			FileServiceImplement* _file_service;
	};
}
