#include <brpc/server.h>
#include <butil/logging.h>

#include "etcd.hpp"
#include "logger.hpp"
#include "channel.hpp"
#include "utils.hpp"
#include "mysql_chat_session.hpp"
#include "mysql_chat_session_member.hpp"
#include "mysql_channel.hpp"

#include "base.pb.h"
#include "channel.pb.h"
#include "transmite.pb.h"

namespace MY_IM{
class ChannelServiceImpl : public MY_IM::ChannelService {
    public:
        ChannelServiceImpl(const std::shared_ptr<odb::core::database> &mysql_client,
            const ServiceManager::ptr &channels,
            const std::string &transmite_service_name):
            _mysql_chat_session(std::make_shared<ChatSessionTable>(mysql_client)),
            _mysql_chat_session_member(std::make_shared<ChatSessionMemeberTable>(mysql_client)),
            _mysql_channel(std::make_shared<ChannelTable>(mysql_client)),
            _mm_channels(channels),
            _transmite_service_name(transmite_service_name){}

        void CreateChannel(google::protobuf::RpcController* controller,
                       const ::MY_IM::CreateChannelReq* request,
                       ::MY_IM::CreateChannelRsp* response,
                       ::google::protobuf::Closure* done) override {
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                const std::string &errmsg) -> void {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            std::string rid = request->request_id();
            std::string uid = request->user_id();
            std::string channel_name = request->channel_name();
            if (uid.empty() || channel_name.empty()) {
                return err_response(rid, "创建频道参数不合法");
            }

            std::string channel_id = Uuid();
            ChatSession cs(channel_id, channel_name, ChatSessionType::CHANNEL);
            bool ret = _mysql_chat_session->insert(cs);
            if (ret == false) {
                LOG_ERROR("{} - 向数据库添加频道会话失败: {}", rid, channel_name);
                return err_response(rid, "向数据库添加频道会话失败");
            }
            Channel channel(channel_id, uid);
            ret = _mysql_channel->insert(channel);
            if (ret == false) {
                LOG_ERROR("{} - 向数据库添加频道信息失败: {}", rid, channel_id);
                return err_response(rid, "向数据库添加频道信息失败");
            }
            std::vector<ChatSessionMember> member_list;
            bool has_creator = false;
            for (int i = 0; i < request->member_id_list_size(); i++) {
                std::string mid = request->member_id_list(i);
                if (mid == uid) {
                    has_creator = true;
                }
                member_list.emplace_back(channel_id, mid);
            }
            if (!has_creator) {
                member_list.emplace_back(channel_id, uid);
            }
            ret = _mysql_chat_session_member->append(member_list);
            if (ret == false) {
                LOG_ERROR("{} - 向数据库添加频道成员失败: {}", rid, channel_id);
                return err_response(rid, "向数据库添加频道成员失败");
            }

            response->set_request_id(rid);
            response->set_success(true);
            response->set_channel_id(channel_id);
        }

        void SendChannelMessage(google::protobuf::RpcController* controller,
                       const ::MY_IM::SendChannelMessageReq* request,
                       ::MY_IM::SendChannelMessageRsp* response,
                       ::google::protobuf::Closure* done) override {
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                const std::string &errmsg) -> void {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            std::string rid = request->request_id();
            std::string uid = request->user_id();
            std::string channel_id = request->channel_id();
            if (uid.empty() || channel_id.empty()) {
                return err_response(rid, "发送频道消息参数不合法");
            }
            std::string creator_id = _mysql_channel->creator(channel_id);
            if (creator_id.empty()) {
                return err_response(rid, "频道不存在");
            }
            if (creator_id != uid) {
                return err_response(rid, "只有频道创建者可以发消息");
            }
            auto channel = _mm_channels->GetChannel(_transmite_service_name);
            if (!channel) {
                LOG_ERROR("{} 未找到可提供业务处理的消息转发子服务节点！", rid);
                return err_response(rid, "未找到消息转发子服务节点");
            }
            MY_IM::MsgTransmitService_Stub stub(channel.get());
            MY_IM::NewMessageReq req;
            MY_IM::GetTransmitTargetRsp rsp;
            req.set_request_id(rid);
            req.set_user_id(uid);
            if (!request->session_id().empty()) {
                req.set_session_id(request->session_id());
            }
            req.set_chat_session_id(channel_id);
            req.mutable_message()->CopyFrom(request->message());
            brpc::Controller cntl;
            stub.GetTransmitTarget(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() || rsp.success() == false) {
                LOG_ERROR("{} 消息转发子服务调用失败！", rid);
                return err_response(rid, "消息转发子服务调用失败");
            }
            response->set_request_id(rid);
            response->set_success(true);
            response->mutable_message()->CopyFrom(rsp.message());
            for (int i = 0; i < rsp.target_id_list_size(); i++) {
                response->add_target_id_list(rsp.target_id_list(i));
            }
        }
    private:
        ChatSessionTable::ptr _mysql_chat_session;
        ChatSessionMemeberTable::ptr _mysql_chat_session_member;
        ChannelTable::ptr _mysql_channel;
        ServiceManager::ptr _mm_channels;
        std::string _transmite_service_name;
};

class ChannelServer {
    public:
        using ptr = std::shared_ptr<ChannelServer>;
        ChannelServer(const Discoverer::ptr &discoverer_client,
            const Registerant::ptr &_register_client,
            const std::shared_ptr<brpc::Server> &server):
            _service_discoverer(discoverer_client),
            _register_client(_register_client),
            _rpc_server(server){}
        void start() {
            _rpc_server->RunUntilAskedToQuit();
        }
    private:
        Discoverer::ptr _service_discoverer;
        Registerant::ptr _register_client;
        std::shared_ptr<brpc::Server> _rpc_server;
};

class ChannelServerBuilder {
    public:
        void make_mysql_object(
            const std::string &user,
            const std::string &pswd,
            const std::string &host,
            const std::string &db,
            const std::string &cset,
            int port,
            int conn_pool_count) {
            _mysql_client = ODBFactory::create(user, pswd, host, db, cset, port, conn_pool_count);
        }
        void make_discoverer_object(const std::string &reg_host,
            const std::string &base_service_name,
            const std::string &transmite_service_name) {
            _transmite_service_name = transmite_service_name;
            _mm_channels = std::make_shared<ServiceManager>();
            _mm_channels->FollowOn(transmite_service_name);
            auto put_cb = std::bind(&ServiceManager::OnlineCall, _mm_channels.get(), std::placeholders::_1, std::placeholders::_2);
            auto del_cb = std::bind(&ServiceManager::OfflineCall, _mm_channels.get(), std::placeholders::_1, std::placeholders::_2);
            _service_discoverer = std::make_shared<Discoverer>(reg_host, base_service_name, put_cb, del_cb);
        }
        void make_registry_object(const std::string &reg_host,
            const std::string &service_name,
            const std::string &access_host) {
            _register_client = std::make_shared<Registerant>(reg_host);
            _register_client->Register(service_name, access_host);
        }
        void make_rpc_server(uint16_t port, int32_t timeout, uint8_t num_threads) {
            if (!_mysql_client) {
                LOG_ERROR("还未初始化Mysql数据库模块！");
                abort();
            }
            if (!_mm_channels) {
                LOG_ERROR("还未初始化信道管理模块！");
                abort();
            }
            _rpc_server = std::make_shared<brpc::Server>();
            ChannelServiceImpl *channel_service = new ChannelServiceImpl(
                _mysql_client, _mm_channels, _transmite_service_name);
            int ret = _rpc_server->AddService(channel_service,
                brpc::ServiceOwnership::SERVER_OWNS_SERVICE);
            if (ret == -1) {
                LOG_ERROR("添加频道服务失败！");
                abort();
            }
            brpc::ServerOptions options;
            options.idle_timeout_sec = timeout;
            options.num_threads = num_threads;
            ret = _rpc_server->Start(port, &options);
            if (ret == -1) {
                LOG_ERROR("频道服务启动失败！");
                abort();
            }
        }
        ChannelServer::ptr build() {
            if (!_service_discoverer || !_register_client || !_rpc_server) {
                LOG_ERROR("频道服务构建依赖未初始化！");
                abort();
            }
            return std::make_shared<ChannelServer>(_service_discoverer, _register_client, _rpc_server);
        }
    private:
        std::shared_ptr<odb::core::database> _mysql_client;
        std::string _transmite_service_name;
        ServiceManager::ptr _mm_channels;
        Discoverer::ptr _service_discoverer;
        Registerant::ptr _register_client;
        std::shared_ptr<brpc::Server> _rpc_server;
};
}
