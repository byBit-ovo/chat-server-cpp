# client 目录：为每个 .cc 分别编译生成网关 HTTP 测试客户端
# 依赖：../server/proto/*.proto、../server/third/include/httplib.h
# 需安装：protobuf-compiler, libprotobuf-dev, libgflags-dev, libssl-dev

CXX       ?= g++
CXXFLAGS  := -std=c++17 -Wall -O2 -g
INC       := -I. -I../server/third/include -Igen
LDFLAGS   := -lgflags -lprotobuf -lssl -lcrypto -lpthread

PROTO_PATH := ../server/proto
PROTO_GEN  := gen
PROTO_FILES := base user friend message transmite file speech channel gateway notify

# 生成的 .pb.cc 和 .pb.o
PROTO_SRCS := $(addprefix $(PROTO_GEN)/,$(addsuffix .pb.cc,$(PROTO_FILES)))
PROTO_OBJS := $(addprefix $(PROTO_GEN)/,$(addsuffix .pb.o,$(PROTO_FILES)))

# 7 个测试客户端源文件及目标名
CLIENT_SRCS := user.cc friend.cc message.cc transmit.cc file.cc speech.cc channel.cc
CLIENT_BINS := $(CLIENT_SRCS:%.cc=client_%)

.PHONY: all clean proto dirs

all: dirs proto $(CLIENT_BINS)

dirs:
	@mkdir -p $(PROTO_GEN)

# 从 server/proto 生成 C++ 代码到 client/gen（base 无依赖，其余 import base）
proto: dirs $(PROTO_SRCS)

$(PROTO_GEN)/base.pb.cc $(PROTO_GEN)/base.pb.h: $(PROTO_PATH)/base.proto
	@mkdir -p $(PROTO_GEN)
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<

$(PROTO_GEN)/user.pb.cc $(PROTO_GEN)/user.pb.h: $(PROTO_PATH)/user.proto | $(PROTO_GEN)/base.pb.cc
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<
$(PROTO_GEN)/friend.pb.cc $(PROTO_GEN)/friend.pb.h: $(PROTO_PATH)/friend.proto | $(PROTO_GEN)/base.pb.cc
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<
$(PROTO_GEN)/message.pb.cc $(PROTO_GEN)/message.pb.h: $(PROTO_PATH)/message.proto | $(PROTO_GEN)/base.pb.cc
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<
$(PROTO_GEN)/transmite.pb.cc $(PROTO_GEN)/transmite.pb.h: $(PROTO_PATH)/transmite.proto | $(PROTO_GEN)/base.pb.cc
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<
$(PROTO_GEN)/file.pb.cc $(PROTO_GEN)/file.pb.h: $(PROTO_PATH)/file.proto | $(PROTO_GEN)/base.pb.cc
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<
$(PROTO_GEN)/speech.pb.cc $(PROTO_GEN)/speech.pb.h: $(PROTO_PATH)/speech.proto
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<
$(PROTO_GEN)/channel.pb.cc $(PROTO_GEN)/channel.pb.h: $(PROTO_PATH)/channel.proto | $(PROTO_GEN)/base.pb.cc
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<
$(PROTO_GEN)/gateway.pb.cc $(PROTO_GEN)/gateway.pb.h: $(PROTO_PATH)/gateway.proto
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<
$(PROTO_GEN)/notify.pb.cc $(PROTO_GEN)/notify.pb.h: $(PROTO_PATH)/notify.proto | $(PROTO_GEN)/base.pb.cc
	protoc --cpp_out=$(PROTO_GEN) -I$(PROTO_PATH) --experimental_allow_proto3_optional $<

# proto .o 只编译，不链接
$(PROTO_GEN)/%.pb.o: $(PROTO_GEN)/%.pb.cc
	$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<

# 每个测试客户端：对应 .cc + 全部 proto .o
client_user: user.cc $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ user.cc $(PROTO_OBJS) $(LDFLAGS)

client_friend: friend.cc $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ friend.cc $(PROTO_OBJS) $(LDFLAGS)

client_message: message.cc $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ message.cc $(PROTO_OBJS) $(LDFLAGS)

client_transmit: transmit.cc $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ transmit.cc $(PROTO_OBJS) $(LDFLAGS)

client_file: file.cc $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ file.cc $(PROTO_OBJS) $(LDFLAGS)

client_speech: speech.cc $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ speech.cc $(PROTO_OBJS) $(LDFLAGS)

client_channel: channel.cc $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ channel.cc $(PROTO_OBJS) $(LDFLAGS)

clean:
	rm -rf $(PROTO_GEN) $(CLIENT_BINS)
