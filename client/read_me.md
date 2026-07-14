# ChatClient 技术文档

> 基于 Qt6 的微信风格即时通讯桌面客户端，使用 Protobuf 协议，HTTP + WebSocket 双通道通信。

---

## 目录

1. [项目概览](#1-项目概览)
2. [模块架构](#2-模块架构)
3. [模块详解](#3-模块详解)
   - [3.1 入口 & 调试配置](#31-入口--调试配置)
   - [3.2 数据模型层 (model/)](#32-数据模型层-model)
   - [3.3 网络通信层 (network/)](#33-网络通信层-network)
   - [3.4 协议层 (proto/)](#34-协议层-proto)
   - [3.5 主窗口 (MainWidget)](#35-主窗口-mainwidget)
   - [3.6 登录模块](#36-登录模块)
   - [3.7 会话 & 好友区](#37-会话--好友区)
   - [3.8 消息展示区](#38-消息展示区)
   - [3.9 消息编辑区](#39-消息编辑区)
   - [3.10 会话详情页](#310-会话详情页)
   - [3.11 群聊详情页](#311-群聊详情页)
   - [3.12 个人信息](#312-个人信息)
   - [3.13 用户信息卡](#313-用户信息卡)
   - [3.14 添加好友](#314-添加好友)
   - [3.15 选择好友](#315-选择好友)
   - [3.16 历史消息搜索](#316-历史消息搜索)
   - [3.17 语音录制](#317-语音录制)
   - [3.18 轻提示](#318-轻提示)
   - [3.19 资源文件](#319-资源文件)
4. [模块关系图](#4-模块关系图)
5. [数据流](#5-数据流)
6. [构建系统](#6-构建系统)

---

## 1. 项目概览

| 属性 | 说明 |
|------|------|
| 语言 | C++17 |
| 框架 | Qt 6.2+ |
| 构建工具 | qmake (主) / CMake (备选) |
| 平台 | macOS (主), 可跨平台 |
| 通信协议 | Protobuf 二进制序列化 |
| 网络通道 | HTTP (请求/响应) + WebSocket (实时推送) |
| 项目文件 | `ChatClient.pro` |

### 依赖的 Qt 模块

| 模块 | 用途 |
|------|------|
| Qt Widgets | GUI 界面框架 |
| Qt Network | HTTP 网络请求 (`QNetworkAccessManager`) |
| Qt WebSockets | WebSocket 实时通信 |
| Qt Protobuf | Protobuf 序列化/反序列化 |
| Qt Multimedia | 语音录制与播放 |

---

## 2. 模块架构

项目采用**三层架构**：

```
┌─────────────────────────────────────────────────────────┐
│                     UI 层 (View)                         │
│  mainwidget / loginwidget / sessionfriendarea /          │
│  messageshowarea / messageeditarea / ...                 │
│  负责界面展示与用户交互，通过信号槽与数据层通信            │
└──────────────────────┬──────────────────────────────────┘
                       │ 调用 Async 方法
                       │ 监听信号 (Done / receiveXxx)
                       ▼
┌─────────────────────────────────────────────────────────┐
│                  数据管理层 (Model)                       │
│              model::DataCenter (单例)                     │
│  管理全局状态: 用户信息、好友列表、会话列表、消息列表等     │
│  持有 NetClient 实例，对外暴露 Async 方法                   │
│  通过 30+ 个信号通知 UI 层数据变化                         │
└──────────────────────┬──────────────────────────────────┘
                       │ 委托网络操作
                       ▼
┌─────────────────────────────────────────────────────────┐
│                  网络通信层 (Network)                      │
│              network::NetClient                           │
│  HTTP:  POST → http://127.0.0.1:8000 (24 个 API)         │
│  WebSocket: ws://127.0.0.1:8001/ws (8 种推送事件)        │
│  序列化/反序列化 Protobuf，模板化响应处理                   │
└─────────────────────────────────────────────────────────┘
```

---

## 3. 模块详解

### 3.1 入口 & 调试配置

#### `main.cpp`

程序入口，初始化 `QApplication`，根据 `debug.h` 中的宏决定启动路径：

| 宏 | 行为 |
|----|------|
| `TEST_SKIP_LOGIN = 1` | 直接打开 MainWidget (跳过登录) |
| `TEST_SKIP_LOGIN = 0` | 先打开 LoginWidget |
| `DEPOLY = 1` | 重定向所有 qDebug 输出到 `./log.txt` 文件 |
| `TEST_NETWORK = 1` | 启动后立即执行 ping 网络连通性测试 |

#### `debug.h`

编译期开关，无需重新编译整个项目即可切换行为：

| 宏 | 默认值 | 作用 |
|----|--------|------|
| `TEST_UI` | 0 | UI 测试模式 |
| `TEST_GROUP_SESSION_DETAIL` | 1 | 群聊详情测试 |
| `TEST_SKIP_LOGIN` | 0 | 跳过登录直接进主窗口 |
| `TEST_NETWORK` | 0 | 启动网络连通性测试 |
| `DEPOLY` | 0 | 部署模式（日志写文件） |

---

### 3.2 数据模型层 (model/)

#### `model/data.h`

定义客户端核心数据结构，位于 `model` 命名空间下。所有结构均有 `load()` 方法从 Protobuf 对象反序列化。

**UserInfo**：用户信息（userId, nickname, description, phone, avatar）

**MessageType 枚举**：
| 值 | 含义 |
|----|------|
| `TEXT_TYPE` | 文本消息 |
| `IMAGE_TYPE` | 图片消息 |
| `FILE_TYPE` | 文件消息 |
| `SPEECH_TYPE` | 语音消息 |

**Message**：消息（messageId, chatSessionId, time, messageType, sender, content, fileId, fileName）。提供静态工厂方法 `makeMessage()` 创建不同类型消息，内部自动生成唯一 ID 和时间戳。

**ChatSessionInfo**：会话信息（chatSessionId, chatSessionName, lastMessage, avatar, userId）。`userId` 在单聊时为对方用户 ID，群聊时为空。

**工具函数**：`TAG`/`LOG()` 调试宏、`formatTime()` 时间格式化、`makeIcon()` 二进制转图标、`loadFileToByteArray()`/`writeByteArrayToFile()` 文件读写。

#### `model/datacenter.h` + `datacenter.cpp`

**单例类**，全局数据中心。UI 层通过它访问数据，网络层通过它存储响应结果。

**持有的数据**：

| 成员 | 类型 | 说明 |
|------|------|------|
| `loginSessionId` | QString | 登录会话 ID（持久化到文件） |
| `myself` | UserInfo* | 当前用户信息 |
| `friendList` | QList\<UserInfo\>* | 好友列表 |
| `chatSessionList` | QList\<ChatSessionInfo\>* | 会话列表 |
| `currentChatSessionId` | QString | 当前选中的会话 |
| `memberList` | QHash\<QString, QList\<UserInfo\>\>* | 群聊成员 (key=chatSessionId) |
| `applyList` | QList\<UserInfo\>* | 待处理好友申请列表 |
| `recentMessages` | QHash\<QString, QList\<Message\>\>* | 每个会话的最近消息 |
| `unreadMessageCount` | QHash\<QString, int\>* | 每个会话未读计数 |
| `searchUserResult` | QList\<UserInfo\>* | 用户搜索结果 |
| `searchMessageResult` | QList\<Message\>* | 消息搜索结果 |
| `currentVerifyCodeId` | QString | 短信验证码 ID |
| `netClient` | network::NetClient | 网络客户端实例 |

**核心方法命名规范**：

```cpp
void getXxxAsync(参数...)              // 异步发起网络请求 (供 UI 调用)
QList<Xxx>* getXxx(参数...)            // 获取本地缓存数据 (供 UI 读取)
void resetXxx(shared_ptr<XxxRsp>)      // 将网络响应写入本地 (供 NetClient 回调)
```

**30+ 个信号**（部分）：

| 信号 | 触发时机 |
|------|----------|
| `getMyselfDone()` | 个人信息获取完成 |
| `getFriendListDone()` | 好友列表获取完成 |
| `getChatSessionListDone()` | 会话列表获取完成 |
| `getRecentMessageListDone(chatSessionId)` | 历史消息加载完成 |
| `sendMessageDone(type, content, extraInfo)` | 消息发送完成 |
| `receiveMessageDone(lastMessage)` | 收到新消息（当前会话） |
| `updateLastMessage(chatSessionId)` | 需要更新会话预览 |
| `userLoginDone(ok, reason)` | 登录完成 |
| `receiveFriendApplyDone()` | 收到新的好友申请 |
| `receiveFriendProcessDone(nickname, agree)` | 好友申请被处理 |
| `receiveSessionCreateDone()` | 收到新会话创建通知 |
| `searchUserDone()` | 用户搜索完成 |
| `searchMessageDone()` | 消息搜索完成 |
| `getSingleFileDone(fileId, content)` | 文件下载完成 |
| `speechConvertTextDone(fileId, text)` | 语音转文字完成 |
| `deleteFriendDone()` | 好友删除完成 |
| `acceptFriendApplyDone()` / `rejectFriendApplyDone()` | 好友申请处理完成 |
| `createGroupChatSessionDone()` | 群聊创建完成 |
| `changeNicknameDone()` / `changeDescriptionDone()` / `changeAvatarDone()` | 个人资料修改完成 |

**数据持久化**：`loginSessionId` 和 `unreadMessageCount` 通过 JSON 持久化到 `QStandardPaths::AppDataLocation/ChatClient.json`，应用重启后恢复。

---

### 3.3 网络通信层 (network/)

#### `network/NetClient.h` + `NetClient.cpp`

负责所有与后端的网络通信，被 `DataCenter` 持有。

**服务器地址配置**（`NetClient.h:25-26`）：

```cpp
const QString HTTP_URL = "http://127.0.0.1:8000";
const QString WEBSOCKET_URL = "ws://127.0.0.1:8001/ws";
```

这是**唯一**需要修改后端地址的地方。

**HTTP 通信**：

统一请求入口 `sendHttpRequest(apiPath, body)`：
- 拼接 `HTTP_URL + apiPath`
- 设置 `Content-Type: application/x-protobuf`
- 执行 HTTP POST

模板化响应处理 `handleHttpResponse<T>(httpResp, &ok, &reason)`：
- 检查 HTTP 错误 → 读取 body → Protobuf 反序列化 → 检查业务 success → 返回结果

每个 API 的标准调用链（5 步）：

```
① 构造 Protobuf 请求对象 → 序列化
② sendHttpRequest(apiPath, body)
③ connect(httpResp, finished, lambda)
④ handleHttpResponse<T>() → 解析响应
⑤ emit dataCenter->xxxDone() → 通知 UI 更新
```

**全部 24 个 HTTP API**：

| 路由 | 方法 | 用途 |
|------|------|------|
| `/service/user/username_login` | `userLogin` | 用户名密码登录 |
| `/service/user/username_register` | `userRegister` | 用户名密码注册 |
| `/service/user/phone_login` | `phoneLogin` | 手机验证码登录 |
| `/service/user/phone_register` | `phoneRegister` | 手机验证码注册 |
| `/service/user/get_user_info` | `getMyself` | 获取个人信息 |
| `/service/user/set_nickname` | `changeNickname` | 修改昵称 |
| `/service/user/set_description` | `changeDescription` | 修改个性签名 |
| `/service/user/get_phone_verify_code` | `getVerifyCode` | 获取短信验证码 |
| `/service/user/set_phone` | `changePhone` | 修改手机号 |
| `/service/user/set_avatar` | `changeAvatar` | 修改头像 |
| `/service/friend/get_friend_list` | `getFriendList` | 获取好友列表 |
| `/service/friend/remove_friend` | `deleteFriend` | 删除好友 |
| `/service/friend/add_friend_apply` | `addFriendApply` | 发送好友申请 |
| `/service/friend/add_friend_process` | `acceptFriendApply` / `rejectFriendApply` | 处理好友申请 |
| `/service/friend/get_pending_friend_events` | `getApplyList` | 获取好友申请列表 |
| `/service/friend/search_friend` | `searchUser` | 搜索用户 |
| `/service/friend/get_chat_session_list` | `getChatSessionList` | 获取会话列表 |
| `/service/friend/create_chat_session` | `createGroupChatSession` | 创建群聊 |
| `/service/friend/get_chat_session_member` | `getMemberList` | 获取群成员 |
| `/service/message_storage/get_recent` | `getRecentMessageList` | 获取最近消息 |
| `/service/message_storage/search_history` | `searchMessage` | 按关键词搜索历史消息 |
| `/service/message_storage/get_history` | `searchMessageByTime` | 按时间搜索历史消息 |
| `/service/message_transmit/new_message` | `sendMessage` | 发送消息 |
| `/service/file/get_single_file` | `getSingleFile` | 下载文件内容 |
| `/service/speech/recognition` | `speechConvertText` | 语音转文字 |

**WebSocket 通信**：

`initWebsocket()`：连接 `ws://127.0.0.1:8001/ws`，连接成功后自动 `sendAuth()` 携带 `loginSessionId` 进行身份认证。收到二进制消息 → 反序列化为 `NotifyMessage` → `handleWsResponse()` 分发。

`handleWsResponse()` 根据 `notifyType` 分发 5 种推送：

| NotifyType | 处理方法 | 推送内容 |
|------------|----------|----------|
| `CHAT_MESSAGE_NOTIFY` | `handleWsMessage` | 新消息 → 加入对应会话的消息列表 |
| `CHAT_SESSION_CREATE_NOTIFY` | `handleWsSessionCreate` | 新群聊被创建 |
| `FRIEND_ADD_APPLY_NOTIFY` | `handleWsAddFriendApply` | 有人申请加好友 |
| `FRIEND_ADD_PROCESS_NOTIFY` | `handleWsAddFriendProcess` | 好友申请被处理(同意/拒绝) |
| `FRIEND_REMOVE_NOTIFY` | `handleWsRemoveFriend` | 被好友删除 |

`handleWsMessage()` 处理两种情况：
- 消息所属会话**已加载** → 直接尾插到消息列表 → 通知 UI
- 消息所属会话**未加载** → 先异步加载消息列表，再接收消息

其他关键方法：`makeRequestId()` 生成 "R" + UUID片段 作为唯一请求 ID；`closeWebsocket()` 主动断开 WebSocket（主窗口关闭时调用）。

---

### 3.4 协议层 (proto/)

位于 `proto/` 目录下，共 **9 套** Protobuf 文件。每套包含 3 类文件：`.qpb.h`（声明）、`.qpb.cpp`（实现）、`*_qtprotoreg.cpp`（类型注册），由 `generate_proto.sh` 脚本调用 `protoc` + `qtprotobufgen` 从 `.proto` 生成。

| Proto 文件 | 定义内容 |
|------------|----------|
| `base` | 基础 RPC 框架（通用 requestId, sessionId, success, errmsg） |
| `user` | 用户信息、登录、注册、修改资料、验证码 |
| `friend` | 好友列表、添加/删除好友、好友申请、会话管理、搜索 |
| `message_storage` | 消息存储、历史消息获取与搜索 |
| `message_transmit` | 消息发送请求、WebSocket 推送 |
| `gateway` | 网关（客户端身份认证） |
| `file` | 文件上传/下载 |
| `notify` | WebSocket 通知消息类型与结构 |
| `speech_recognition` | 语音识别请求/响应 |

---

### 3.5 主窗口 (MainWidget)

#### `mainwidget.h` + `mainwidget.cpp` + `mainwidget.ui`

**单例类**，应用主界面，微信风格**三栏布局**：

```
┌──────────┬──────────────────────┬──────────┐
│  左侧栏   │       中间栏          │  右侧栏   │
│ (windowLeft) │    (windowMid)    │(windowRight)│
│          │                      │          │
│ 用户头像  │  会话标题 + 详情按钮   │ 消息展示区 │
│ 会话Tab  │                      │(Message   │
│ 好友Tab  │                      │ ShowArea) │
│ 申请Tab  │                      │          │
│ 搜索框   │                      │ 消息编辑区 │
│ 添加好友  │                      │(Message   │
│          │                      │ EditArea) │
│ 会话/好友 │                      │          │
│ /申请列表│                      │          │
│(Session  │                      │          │
│ Friend   │                      │          │
│ Area)    │                      │          │
└──────────┴──────────────────────┴──────────┘
```

**关键成员**：

| 成员 | 说明 |
|------|------|
| `userAvatar` | 用户头像按钮（点击打开个人信息页） |
| `sessionTabBtn` / `friendTabBtn` / `applyTabBtn` | 三个内容 Tab |
| `searchEdit` | 搜索输入框 |
| `addFriendBtn` | 添加好友按钮 |
| `sessionFriendArea` | 会话/好友/申请列表容器 |
| `sessionTitleLabel` | 当前会话标题 |
| `extraBtn` | 会话详情按钮 |
| `messageShowArea` | 消息气泡展示区 |
| `messageEditArea` | 消息编辑区 |
| `activeTab` | 当前激活的 Tab 枚举（SESSION_LIST / FRIEND_LIST / APPLY_LIST） |

**核心方法**：

| 方法 | 说明 |
|------|------|
| `initMainWindow()` | 初始化主窗口布局、大小、样式 |
| `initLeftWindow()` | 构建左侧栏（头像、Tab、搜索框、SessionFriendArea） |
| `initMidWindow()` | 构建中间栏（会话标题、MessageShowArea） |
| `initRightWindow()` | 构建右侧栏（MessageEditArea） |
| `initSignalSlot()` | 连接所有信号槽（DataCenter 信号 → UI 更新） |
| `initWebsocket()` | 初始化 WebSocket 连接 |
| `switchTabToSession()` / `switchTabToFriend()` / `switchTabToApply()` | 切换左侧 Tab 并加载对应数据 |
| `loadSessionList()` / `loadFriendList()` / `loadApplyList()` | 异步加载各列表（优先本地缓存） |
| `switchSession(userId)` | 点击好友后切换到对应会话 |
| `loadRecentMessage(chatSessionId)` | 加载并显示指定会话的最近消息 |
| `updateRecentMessage(chatSessionId)` | 更新指定会话的消息预览 |
| `closeEvent(event)` | 窗口关闭时主动断开 WebSocket |

---

### 3.6 登录模块

#### `loginwidget.h` + `loginwidget.cpp`

**用户名密码登录/注册**界面。包含用户名输入框、密码输入框、验证码输入区（含 `VerifyCodeWidget`）。

关键方法：
- `switchMode()` — 切换登录/注册模式
- `clickSubmitBtn()` — 校验输入后调用 `DataCenter::userLoginAsync()` 或 `userRegisterAsync()`
- `userLoginDone(ok, reason)` — 登录成功则创建并显示 `MainWidget`

#### `phoneloginwidget.h` + `phoneloginwidget.cpp`

**手机验证码登录/注册**界面。包含手机号输入框、验证码输入框、"获取验证码"按钮（30 秒倒计时）。

关键方法：
- `sendVerifyCode()` — 发送手机号到服务器并开始倒计时
- `clickSubmitBtn()` — 调用 `DataCenter::phoneLoginAsync()` 或 `phoneRegisterAsync()`
- `countDown()` — 倒计时按钮状态管理
- `switchMode()` — 切换登录/注册模式

#### `verifycodewidget.h` + `verifycodewidget.cpp`

**图形验证码**组件。`paintEvent()` 绘制随机 4 位大写字母 + 随机颜色 + 噪点 + 干扰线，实现 captcha 风格的视觉混淆。点击刷新验证码。

关键方法：
- `generateVerifyCode()` — 生成随机字符串
- `refreshVerifyCode()` — 重新生成并触发重绘
- `checkVerifyCode(code)` — 不区分大小写比较

---

### 3.7 会话 & 好友区

#### `sessionfriendarea.h` + `sessionfriendarea.cpp`

`QScrollArea` 中的垂直列表，根据 `MainWidget::activeTab` 显示三类内容：

| Tab | 显示内容 | 单项组件 | 数据来源 | 点击行为 |
|-----|----------|----------|----------|----------|
| 会话列表 | 头像 + 名称 + 最后消息预览 + 未读数 | `SessionItem` | `DataCenter::chatSessionList` | 加载该会话的消息 |
| 好友列表 | 头像 + 昵称 + 签名 | `FriendItem` | `DataCenter::friendList` | `MainWidget::switchSession()` 切到对方会话 |
| 申请列表 | 头像 + 昵称 + 同意/拒绝按钮 | `ApplyItem` | `DataCenter::applyList` | 同意 → `acceptFriendApplyAsync()` / 拒绝 → `rejectFriendApplyAsync()` |

提供 `clear()` 清除所有项，`addItem()` 添加列表项。基类 `SessionFriendItem` 管理通用布局（头像 + 名称 + 预览）、单选高亮和悬停效果。

---

### 3.8 消息展示区

#### `messageshowarea.h` + `messageshowarea.cpp`

`QScrollArea` 中的消息气泡展示区，支持 4 种消息渲染：

| 消息类型 | 渲染组件 | 展示形式 |
|----------|----------|----------|
| 文本 | `MessageContentLabel` | 圆角矩形气泡 + 三角箭头 + 文字 |
| 图片 | `MessageImageLabel` | 气泡 + 缩放缩略图（点击查看大图） |
| 文件 | `MessageContentLabel` | 气泡 + 文件名 + 文件图标（点击下载） |
| 语音 | `MessageContentLabel` | 气泡 + 播放图标 + 时长（右键菜单转文字） |

消息区分左右：自己发送的靠右蓝色气泡，他人发送的靠左白色气泡。

`MessageItem` 是工厂类，根据消息类型创建对应内容组件。提供 `addMessage()`（尾插）、`addFrontMessage()`（头插，用于加载更早消息）、`clear()`、`scrollToEnd()` 等方法。

---

### 3.9 消息编辑区

#### `messageeditarea.h` + `messageeditarea.cpp`

底部消息输入编辑区，提供完整的消息发送能力：

| 功能 | 实现 |
|------|------|
| 文本输入 | `QTextEdit` 多行输入框 |
| 发送文本 | Enter 键 → `DataCenter::sendTextMessageAsync()` |
| 发送图片 | 工具栏按钮 → 文件选择器 → `DataCenter::sendImageMessageAsync()` |
| 发送文件 | 工具栏按钮 → 文件选择器 → `DataCenter::sendFileMessageAsync()` |
| 语音录制 | 工具栏按钮长按 → `SoundRecorder` → `DataCenter::sendSpeechMessageAsync()` |
| 历史消息 | 工具栏按钮 → 打开 `HistoryMessageWidget` |

关键方法：`sendTextMessage()`、`clickSendImageBtn()`、`clickSendFileBtn()`、`soundRecordPressed()`/`soundRecordReleased()`。

---

### 3.10 会话详情页

#### `sessiondetailwidget.h` + `sessiondetailwidget.cpp`

**单聊**会话详情 `QDialog`，显示对方头像、昵称、签名、手机号。提供"删除好友"按钮（调用 `DataCenter::deleteFriendAsync()`）和"创建群聊"入口（通过 `AvatarItem` 触发）。

---

### 3.11 群聊详情页

#### `groupsessiondetailwidget.h` + `groupsessiondetailwidget.cpp`

**群聊**会话详情 `QDialog`，显示群名、群头像、成员网格（使用 `AvatarItem` 组件，每行 4 个）、"添加成员"按钮（调用 `DataCenter::getMemberListAsync()` 获取成员列表）、"退出群聊"按钮。

关键方法：`initData()` 从服务器获取成员列表，`initMembers()` 将成员填充到网格布局。

---

### 3.12 个人信息

#### `selfinfowidget.h` + `selfinfowidget.cpp`

当前用户**个人信息编辑** `QDialog`，每个字段有查看/编辑两种模式（点击切换）：

| 字段 | 修改方法 |
|------|----------|
| 头像 | 点击 → 文件选择器 → `DataCenter::changeAvatarAsync()` |
| 昵称 | 编辑 → `DataCenter::changeNicknameAsync()` |
| 签名 | 编辑 → `DataCenter::changeDescriptionAsync()` |
| 手机号 | 编辑 → `getVerifyCodeAsync()` + `changePhoneAsync()`（含 30 秒倒计时） |

从主窗口点击左上角头像打开。

---

### 3.13 用户信息卡

#### `userinfowidget.h` + `userinfowidget.cpp`

查看**好友信息**的 `QDialog`（只读），显示好友头像、userId、昵称、手机号。

根据好友关系动态控制三个按钮的启用状态：
- "加好友"（非好友时启用）→ `DataCenter::addFriendApplyAsync()`
- "发消息"（好友时启用）→ `MainWidget::switchSession()`
- "删除好友"（好友时启用）→ `DataCenter::deleteFriendAsync()`

---

### 3.14 添加好友

#### `addfrienddialog.h` + `addfrienddialog.cpp`

添加好友 `QDialog`：搜索框 → `DataCenter::searchUserAsync(keyword)` → 显示搜索结果（头像 + 昵称 + 签名 + "添加好友"按钮）。点击添加按钮 → `DataCenter::addFriendApplyAsync(userId)` 发送好友申请。

---

### 3.15 选择好友

#### `choosefrienddialog.h` + `choosefrienddialog.cpp`

多选好友 `QDialog`（用于创建群聊）：左侧好友列表（带 checkbox），右侧已选好友列表（可移除），至少选 2 人（加上自己共 3 人）才能创建群聊。确定 → `DataCenter::createGroupChatSessionAsync(userIdList)`。

---

### 3.16 历史消息搜索

#### `historymessagewidget.h` + `historymessagewidget.cpp`

历史消息搜索 `QDialog`，提供**两种搜索模式**（RadioButton 切换）：

| 模式 | 调用 |
|------|------|
| 按关键词搜索 | `DataCenter::searchMessageAsync(keyword)` |
| 按时间范围搜索 | `DataCenter::searchMessageByTimeAsync(begTime, endTime)`（含 `QDateTimeEdit` 选择器） |

`HistoryItem::makeHistoryItem()` 工厂方法根据消息类型创建对应展示组件：`ImageButton`（图片）、`FileLabel`（文件，点击下载）、`SpeechLabel`（语音，点击播放）。

---

### 3.17 语音录制

#### `soundrecorder.h` + `soundrecorder.cpp`

**单例类**，基于 Qt Multimedia 的语音录制与播放。使用 PCM 格式（16000 Hz, Mono, 16-bit）。

| 方法 | 说明 |
|------|------|
| `startRecord()` | 打开临时文件，开始录制 |
| `stopRecord()` | 停止录制 → emit `soundRecordDone(path)` |
| `startPlay(content)` | 写入临时文件并播放 |
| `stopPlay()` | 停止播放 → emit `soundPlayDone()` |

在 `MessageEditArea` 中通过工具栏语音按钮长按触发。

---

### 3.18 轻提示

#### `toast.h` + `toast.cpp`

`Toast` 是无边框 `QDialog`，在屏幕底部居中显示短通知消息。静态方法 `showMessage(text)` 创建实例并显示，2 秒后自动关闭。

---

### 3.19 资源文件

#### `resource.qrc`

Qt 资源文件，将 `resource/image/` 下的 23 个图片资源打包进可执行文件。通过 `:/resource/image/xxx.png` 路径引用：

| 资源 | 用途 |
|------|------|
| `session_active/inactive.png` | 会话 Tab 图标（两种状态） |
| `friend_active/inactive.png` | 好友 Tab 图标 |
| `apply_active/inactive.png` | 申请 Tab 图标 |
| `defaultAvatar.png` | 默认用户头像 |
| `groupAvatar.png` | 默认群聊头像 |
| `file.png` | 文件消息图标 |
| `image.png` | 图片消息图标 |
| `sound.png` / `sound_active.png` | 语音消息图标 |
| `search.png` | 搜索图标 |
| `more.png` | 更多操作图标 |
| `logo.png` | 应用 Logo |
| `checked.png` / `unchecked.png` | 复选框状态图标 |
| `cross.png` | 关闭/删除图标 |
| `modify.png` | 编辑图标 |
| `submit.png` | 提交图标 |
| `history.png` | 历史消息图标 |

---

## 4. 模块关系图

```
┌────────────────────────────────────────────────────────────────────┐
│                           main.cpp                                 │
│                    (程序入口，创建 QApplication)                      │
└────────────┬──────────────────────────────────┬────────────────────┘
             │ TEST_SKIP_LOGIN=0                │ TEST_SKIP_LOGIN=1
             ▼                                  ▼
    ┌────────────────┐              ┌────────────────────┐
    │  LoginWidget    │              │    MainWidget       │
    │  PhoneLoginWidget│             │    (主窗口，单例)     │
    │  VerifyCodeWidget│             │                     │
    └───────┬────────┘              │  持有并管理:         │
            │ 登录成功               │  ├ SessionFriendArea │
            │ emit userLoginDone    │  ├ MessageShowArea   │
            ▼                       │  └ MessageEditArea   │
    ┌────────────────────┐         │    (含 SoundRecorder)│
    │   MainWidget        │◀────────└──────────┬─────────┘
    └────────┬───────────┘                      │
             │ 持有 & 调用 Async 方法            │ 信号槽监听
             ▼                                  │
    ┌────────────────────┐                      │
    │  model::DataCenter  │◀─────────────────────┘
    │     (单例)          │
    │                     │
    │  管理全局数据:        │     持有 & 委托
    │  - myself           │
    │  - friendList       │
    │  - chatSessionList  │
    │  - recentMessages   │
    │  - applyList        │
    │  - memberList       │
    │  - unreadCount      │
    │                     │
    │  持有 NetClient:     │
    │  network::NetClient │
    └────────┬───────────┘
             │ 持有 & 调用
             ▼
    ┌────────────────────┐
    │ network::NetClient  │
    │                     │
    │  服务器地址:         │
    │  HTTP: 127.0.0.1:8000   │──── POST Protobuf ────▶ 后端服务器
    │  WS:   127.0.0.1:8001   │◀─── 实时推送 NotifyMsg ── 后端服务器
    └────────────────────┘
```

### 文件依赖关系

```
main.cpp
  ├── debug.h (编译开关)
  ├── mainwidget.h
  │     ├── sessionfriendarea.h
  │     ├── messageshowarea.h
  │     └── messageeditarea.h
  │           └── soundrecorder.h
  ├── loginwidget.h
  │     ├── phoneloginwidget.h
  │     └── verifycodewidget.h
  └── model/datacenter.h
        ├── model/data.h
        │     └── proto/*.qpb.h (9 套)
        └── network/NetClient.h
              └── proto/*.qpb.h (9 套)
```

---

## 5. 数据流

### 5.1 HTTP 请求/响应流程（以发送文本消息为例）

```
用户点击发送按钮
  │
  ▼
MessageEditArea
  │  DataCenter::sendTextMessageAsync(chatSessionId, content)
  ▼
DataCenter (datacenter.cpp:302)
  │  netClient.sendMessage(loginSessionId, chatSessionId, TEXT_TYPE, content, "")
  ▼
NetClient (NetClient.cpp:404)
  │  ① 构造 NewMessageReq (Protobuf)
  │  ② 序列化 → QByteArray
  │  ③ sendHttpRequest("/service/message_transmit/new_message", body)
  │  ④ httpClient.post("http://127.0.0.1:8000/service/message_transmit/new_message", body)
  ▼
后端处理...
  │
  ▼
QNetworkReply::finished 信号
  │  ⑤ handleHttpResponse<NewMessageRsp>(resp, &ok, &reason)
  │  ⑥ 反序列化 + 检查 success
  │  ⑦ emit dataCenter->sendMessageDone(type, content, extraInfo)
  ▼
MessageEditArea 收到信号 → 清空输入框 → MessageShowArea 显示新消息气泡
```

### 5.2 WebSocket 推送流程（以收到新消息为例）

```
后端服务器推送 NotifyMessage (二进制)
  │
  ▼
websocketClient 收到 binaryMessageReceived 信号
  │  ① 反序列化 → NotifyMessage
  │  ② handleWsResponse(notifyMessage)
  ▼
notifyType == CHAT_MESSAGE_NOTIFY
  │  ③ Message::load(notifyMessage.newMessageInfo().messageInfo())
  │  ④ handleWsMessage(message)
  ▼
判断消息所属会话是否已加载:
  ├── 已加载 → 尾插到 recentMessages[chatSessionId]
  │              → emit dataCenter->receiveMessageDone(message)
  │              → MessageShowArea 渲染新气泡
  │              → emit dataCenter->updateLastMessage(chatSessionId)
  │              → SessionFriendArea 更新预览
  └── 未加载 → 先异步加载消息列表
                 → emit getRecentMessageListDoneNoUI(chatSessionId)
                 → 加载完成后尾插
```

### 5.3 登录流程

```
LoginWidget 点击登录
  │
  ▼
DataCenter::userLoginAsync(username, password)
  │
  ▼
NetClient::userLogin(username, password)
  │  POST /service/user/username_login
  ▼
收到响应 → loginSessionId = "testLoginSessionId"
  │  DataCenter::resetLoginSessionId(loginSessionId) → 持久化到 ChatClient.json
  │  emit dataCenter->userLoginDone(true, "")
  ▼
LoginWidget 收到信号:
  │  MainWidget::getInstance() → 创建主窗口
  │    ├── initWebsocket() → NetClient::initWebsocket()
  │    └── loadSessionList() / loadFriendList() / loadApplyList()
  │  close LoginWidget
  │  show MainWidget
```

---

## 6. 构建系统

### 6.1 构建方式

```bash
# 一键构建
./build.sh              # Release 构建
./build.sh debug        # Debug 构建
./build.sh run          # 构建并运行
./build.sh clean        # 清理构建产物

# 分步构建
mkdir -p build && cd build
qmake6 ../ChatClient.pro   # 运行 qmake 生成 Makefile
make -j$(nproc)            # 编译
./ChatClient               # 运行
```

### 6.2 Proto 文件生成

```bash
./generate_proto.sh
# 本质调用：
# protoc --plugin=protoc-gen-qtprotobuf=<qtprotobufgen路径> \
#        --qtprotobuf_out=proto/ -Iproto/ proto/*.proto
```

### 6.3 ChatClient.pro 项目文件

```qmake
QT       += core gui widgets network websockets multimedia
CONFIG   += c++17
CONFIG   -= app_bundle    # macOS: 不生成 .app bundle，生成独立可执行文件

SOURCES += ...  # 28 个 .cpp 文件 (含 9 个 proto .qpb.cpp)
HEADERS += ...  # 20 个 .h 文件 (含 9 个 proto .qpb.h)
FORMS   += mainwidget.ui
RESOURCES += resource.qrc
```

### 6.4 文件统计

| 类别 | 数量 | 说明 |
|------|------|------|
| 自定义 `.cpp` 源文件 | 17 | UI 组件 + Model + Network + main |
| 自定义 `.h` 头文件 | 11 | UI 组件 + Model + Network + debug |
| proto `.qpb.h/.cpp` | 各 9 (共 18) | 由 protoc 自动生成 |
| proto `.proto` | 9 | 原始协议定义 |
| `.ui` UI 文件 | 1 | mainwidget.ui (Qt Designer) |
| `.qrc` 资源文件 | 1 | 图片资源索引 |
| 构建脚本 | 2 | build.sh, generate_proto.sh |
