// 服务器对客户端groupmenu里不同的选项进行不同的处理
#ifndef GROUPPROCESS_H
#define GROUPPROCESS_H

#include "../others/data.h"
#include "../others/define.h"
#include "../others/IO.h"
#include "../others/redis.h"
#include <vector>

#include <iostream>
using json = nlohmann::json;
using namespace std;

// 创建群组
void creatgroup_server(int fd, string buf);

// 加入群组(通知管理员)
void addgroup_server(int fd, string buf);

// 查看已加入的群组
void checkgroup_server(int fd, string buf);

// 退出已加入的群组（就不通知管理员了）
void outgroup_server(int fd, string buf);

//  查看群组成员列表
void checkgroupnum_server(int fd, string buf);

// 查看群组聊天记录
void historygroupchat_server(int fd, string buf);

// 选择群组聊天
void groupchat_server(int fd, string buf);

#endif