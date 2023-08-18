// 客户端的群聊选项界面，及与群聊有关的事务
#ifndef GROUPMENU_HPP
#define GROUPMENU_HPP

#include "../others/data.h"
#include "../others/define.h"
#include "../others/head.h"
#include "../others/IO.h"
#include "menu.h"
#include "../others/threadwork.h"
#include "managegroupmenu.h"

#include <iostream>
using json = nlohmann::json;
using namespace std;

void groupmenuUI(void);

// 创建群组
void creatgroup_client(int client_socket, string id, Queue<string> &RecvQue);

// 加入群组
void addgroup_client(int client_socket, string id, Queue<string> &RecvQue);

// 查看已加入的群组
int checkgroup_client(int client_socket, string id, Queue<string> &RecvQue, int fl);

// 退出已加入的群组
void outgroup_client(int client_socket, string id, Queue<string> &RecvQue);

// 查看群组成员列表
string checkgroupnum_client(int client_socket, string id, Queue<string> &RecvQue, int fl);

// 管理群组
void managegroup_client(int client_socket, string id, Queue<string> &RecvQue);

// 查看群组聊天记录
string historygroupchat_client(int client_socket, string id, Queue<string> &RecvQue, int fl);

// 选择群组聊天
void groupchat_client(int client_socket, string id, Queue<string> &RecvQue);


#endif
