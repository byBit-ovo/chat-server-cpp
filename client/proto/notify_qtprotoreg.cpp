
#include "notify.qpb.h"

#include <QtProtobuf/qprotobufregistration.h>

namespace bite_im {
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarNotifyFriendAddApply(qRegisterProtobufType<NotifyFriendAddApply>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarNotifyFriendAddProcess(qRegisterProtobufType<NotifyFriendAddProcess>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarNotifyFriendRemove(qRegisterProtobufType<NotifyFriendRemove>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarNotifyNewChatSession(qRegisterProtobufType<NotifyNewChatSession>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarNotifyNewMessage(qRegisterProtobufType<NotifyNewMessage>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarNotifyMessage(qRegisterProtobufType<NotifyMessage>);
static bool RegisterNotifyProtobufTypes = [](){ qRegisterProtobufTypes(); return true; }();
} // namespace bite_im

