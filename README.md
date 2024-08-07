# 项目构建及运行
0. 前置安装：
```bash
hiredis
nlohmann-json
```
1. 在项目`MyChatroom`目录下打开终端，执行以下命令进行构建：
```bash
mkdir build
cd build
cmake ..
make
```
2. 运行程序：
```bash
./server 监听地址 端口号   // 服务器
./client 监听地址 端口号   // 客户端，可在不同电脑下运行多个
```

> 注：本项目用到`Redis`实现数据存储
> Redis数据库可视化辅助工具：[RedisInsight](https://redis.io/download/)

---
# 目录结构
```
> tree ./MyChatroom/
./MyChatroom/
├── client
│   ├── client.cc
│   ├── filemenu.cc
│   ├── filemenu.h
│   ├── groupmenu.cc
│   ├── groupmenu.h
│   ├── managegroupmenu.cc
│   ├── managegroupmenu.h
│   ├── menu.cc
│   ├── menu.h
│   ├── personalmenu.cc
│   └── personalmenu.h
├── CMakeLists.txt
├── others
│   ├── data.cc
│   ├── data.h
│   ├── define.h
│   ├── head.h
│   ├── IO.cc
│   ├── IO.h
│   ├── queue.hpp
│   ├── redis.cc
│   ├── redis.h
│   ├── threadpool.cc
│   ├── threadpool.h
│   ├── threadwork.cc
│   └── threadwork.h
└── server
    ├── fileprocess.cc
    ├── fileprocess.h
    ├── groupprocess.cc
    ├── groupprocess.h
    ├── login.cc
    ├── login.h
    ├── managegroupprocess.cc
    ├── managegroupprocess.h
    ├── personalprocess.cc
    ├── personalprocess.h
    └── server.cc

4 directories, 36 files
```

---
# 实现功能
## 账号管理
- 实现登录、注册、注销
- 实现找回密码
- 实现数据加密
## 好友管理
- 实现好友的添加、删除、查询操作
- 实现显示好友在线状态
- 禁止不存在好友关系的用户间的私聊
- 实现屏蔽好友消息
- 实现好友间聊天
## 群管理
- 实现群组的创建、解散
- 实现用户申请加入群组
- 实现用户查看已加入的群组
- 实现群组成员退出已加入的群组
- 实现群组成员查看群组成员列表
- 实现群主对群组管理员的添加和删除
- 实现群组管理员批准用户加入群组
- 实现群组管理员/群主从群组中移除用户
- 实现群组内聊天功能
## 聊天功能
对于**私聊和群组**的聊天功能**均实现**：
- 实现在线发送文件
- 实现查看历史消息记录
- 实现用户间在线聊天
- 实现在线用户对离线用户发送文件，离线用户上线后获得通知/接收
- 实现在线用户对离线用户发送消息，离线用户上线后获得通知
- 实现用户在线时,消息的实时弹窗通知
    - 收到好友请求
    - 收到私聊
    - 收到加群申请
    - ...
## 其他
- Epoll ET 模式
- Redis 实现数据存储
- 绑定键盘快捷键，实现用户快捷操作
- C/S 双端均支持在 CLI/Web 自行指定 IP:Port
- 实现服务器日志，记录服务器的状态信息
- 实现具有高稳定性的客户端和服务器，防止在用户非法输入时崩溃或异常
