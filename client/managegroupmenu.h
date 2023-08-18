// 客户端的群聊管理有关的事务
#ifndef MANAGEGROUPMENU_H
#define MANAGEGROUPMENU_H

#include "../others/data.h"
#include "../others/define.h"
#include "../others/head.h"
#include "../others/IO.h"
#include "menu.h"
#include "../others/threadwork.h"
#include "groupmenu.h"

#include <iostream>
using json = nlohmann::json;
using namespace std;

string checkgroupnum_client(int client_socket, string id, Queue<string> &RecvQue, int fl);
int checkgroup_client(int client_socket, string id, Queue<string> &RecvQue, int fl);

void manegegroupUI(void);

// 添加管理员
void addmin_client(int client_socket, string id, Queue<string> &RecvQue);

// 删除群管理员
void deladmin_client(int client_socket, string id, Queue<string> &RecvQue);

// 查看申请加群列表(包括同意请求一起写，但服务器那边要有不同的函数来处理)
void checkapplylist_client(int client_socket, string id, Queue<string> &RecvQue, int fl);

// 删除群成员
void delgroupnum_client(int client_socket, string id, Queue<string> &RecvQue);

// 解散群组
void delgroup_client(int client_socket, string id, Queue<string> &RecvQue);

#endif