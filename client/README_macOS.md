# ChatClient - macOS Build Guide

这是一个基于 Qt6 的微信风格即时通讯客户端。

## 前置条件

### 1. 安装 Qt6 SDK

```bash
# 使用 Homebrew 安装 (推荐)
brew install qt6

# 或者从官网下载: https://www.qt.io/download
```

### 2. 安装后设置环境变量

```bash
# 将 Qt6 的 bin 目录加到 PATH (Apple Silicon Mac)
echo 'export PATH="/opt/homebrew/opt/qt6/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc

# Intel Mac 使用这个路径:
# export PATH="/usr/local/opt/qt6/bin:$PATH"
```

## 快速构建

### 方法一: 使用构建脚本 (推荐)

```bash
cd code/client/ChatClient

# 构建 (release)
./build.sh

# 调试构建
./build.sh debug

# 构建并运行
./build.sh run

# 清理构建
./build.sh clean
```

### 方法二: 使用 qmake 手动构建

```bash
cd code/client/ChatClient

# 1. 生成 protobuf C++ 文件 (如果 Qt 版本 < 6.5 或不支持 qmake protobuf)
./generate_proto.sh

# 2. 创建构建目录
mkdir -p build && cd build

# 3. 运行 qmake
qmake6 ../ChatClient.pro

# 或者如果 qmake6 不在 PATH:
/opt/homebrew/opt/qt6/bin/qmake ../ChatClient.pro

# 4. 编译
make -j$(sysctl -n hw.ncpu)

# 5. 运行
./ChatClient
```

### 方法三: 使用 CMake (备选)

```bash
cd code/client/ChatClient
mkdir -p build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt6
make -j$(sysctl -n hw.ncpu)
```

## 项目结构

```
ChatClient/
├── ChatClient.pro          # qmake 项目文件
├── CMakeLists.txt          # CMake 项目文件 (备选)
├── build.sh                # 一键构建脚本
├── generate_proto.sh       # Protobuf 文件生成脚本
├── main.cpp                # 程序入口
├── mainwidget.*            # 主窗口
├── mainwidget.ui           # Qt UI 文件
├── resource.qrc            # Qt 资源文件
├── model/                  # 数据模型
│   ├── data.h              # 核心数据结构定义
│   └── datacenter.*        # 数据中心 (单例)
├── network/                # 网络通信
│   └── NetClient.*         # HTTP + WebSocket 客户端
├── proto/                  # Protobuf 协议定义
│   ├── base.proto
│   ├── user.proto
│   ├── friend.proto
│   ├── file.proto
│   ├── gateway.proto
│   ├── message_storage.proto
│   ├── message_transmit.proto
│   ├── notify.proto
│   └── speech_recognition.proto
└── resource/image/         # 图片资源
```

## 依赖的 Qt 模块

| 模块 | 用途 |
|------|------|
| Qt Widgets | UI 界面 |
| Qt Network | HTTP 请求 |
| Qt WebSockets | WebSocket 实时通信 |
| Qt Protobuf | Protobuf 序列化 |
| Qt Multimedia | 语音录制/播放 |

## 常见问题

### Q: 提示 "qtprotobufgen not found"
A: Qt6 安装不完整，请确保通过 Homebrew 安装：
```bash
brew install qt6
```

### Q: 编译时找不到 qpb.h 文件
A: 需要先生成 protobuf 文件：
```bash
./generate_proto.sh
```

### Q: 运行时提示找不到 Qt 库
A: 设置动态库搜索路径：
```bash
export DYLD_LIBRARY_PATH="/opt/homebrew/opt/qt6/lib:$DYLD_LIBRARY_PATH"
```

### Q: 应用需要连接后端服务器
A: 该客户端需要配合后端服务器使用。服务器地址在 `network/NetClient.h` 中配置：
- HTTP: `http://127.0.0.1:8000`
- WebSocket: `ws://127.0.0.1:8001/ws`

## 配置调试选项

编辑 `debug.h` 文件：

```cpp
// 跳过登录，直接进入主窗口
#define TEST_SKIP_LOGIN 1

// 启用部署模式（写日志到文件）
#define DEPOLY 1
```
