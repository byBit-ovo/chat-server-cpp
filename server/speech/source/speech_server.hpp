#include "../common/asr.hpp"
#include "../common/etcd.hpp"
#include "speech.pb.h"

#include <brpc/server.h>
#include <butil/logging.h>
#include <gflags/gflags.h>

namespace MY_IM
{
	class SpeechServiceImplement : public SpeechService
	{
	public:
		using ptr = std::shared_ptr<SpeechServiceImplement>;
		SpeechServiceImplement(ASRClient::ptr client):_client(client)
		{}
		virtual void SpeechRecognition(
			google::protobuf::RpcController *controller, 
			const SpeechRecognitionReq *request,
			SpeechRecognitionRsp *response, 
			::google::protobuf::Closure *done) override
		{
			brpc::ClosureGuard guard(done);
			std::string err_msg;
			std::string text  = _client->Recognize(request->speech_content(),err_msg);
			response->set_request_id(request->request_id());
			if(text.empty() == false)
			{
				LOG_ERROR("Speech convert fail,Message-id: {}",request->request_id());
				response->set_errmsg(err_msg);
				response->set_success(false);
				return;
			}
			response->set_success(true);
			response->set_recognition_result(text);
		}
	private:
		ASRClient::ptr _client;
	};

	class SpeechServer
	{
		public:
			using ptr = std::shared_ptr<SpeechServer>;
			SpeechServer(
				std::shared_ptr<brpc::Server> rpc_server,
				Registerant::ptr register_client,
				SpeechServiceImplement::ptr speech_service):
				_rpc_server(rpc_server),
				_register_client(register_client),
				_speech_service(speech_service)
			{}
			void Start()
			{
				_rpc_server->RunUntilAskedToQuit();
			}
		private:
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			SpeechServiceImplement::ptr _speech_service;
			
	};

	class SpeechServerBuilder
	{
		public:
			using ref = SpeechServerBuilder&;
			ref Construct_Register(
				const std::string &etcd_host,
				const std::string &service_name,
				const std::string &service_addr
			)
			{
				_register_client = std::make_shared<Registerant>(etcd_host);
				bool ret = _register_client->Register(service_name,service_addr);
				if(ret == false)
				{
					LOG_ERROR("Register service({}) fail,exit", service_name);
					exit(1);
				}
				return *this;
			}
			ref Construct_ASR_Service(
				const std::string &app_id,
				const std::string &api_key,
				const std::string &secret_key)
				{
					auto asr_client = std::make_shared<ASRClient>(app_id,api_key,secret_key);
					_speech_service = std::make_shared<SpeechServiceImplement>(asr_client);
					return *this;
				}
				  
			ref Construct_Rpc_Server(int port,int ttl,int thread_nums)
			{
				if(_register_client.get() == nullptr || _speech_service.get() == nullptr){
					LOG_WARNING("You should implement service and register before construct rpc server");
					return *this;
				}

				_rpc_server = std::make_shared<brpc::Server>();
				if(-1 == _rpc_server->AddService(_speech_service.get(),brpc::ServiceOwnership::SERVER_DOESNT_OWN_SERVICE))
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

			SpeechServer::ptr Build()
			{
				if(_register_client.get() == nullptr || 
					_speech_service.get() == nullptr || 
					_rpc_server.get()==nullptr)
				{
					LOG_WARNING("You should implement service&register*rpc_server before build SpeechServer!");
					abort();
				}
				return std::make_shared<SpeechServer>(_rpc_server,_register_client,_speech_service);
			}

		private:
			std::shared_ptr<brpc::Server> _rpc_server;
			Registerant::ptr _register_client;
			SpeechServiceImplement::ptr _speech_service;
	};
}