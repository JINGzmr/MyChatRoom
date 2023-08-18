// 客户端的聊天室界面，以及处理好友相关事件
#ifndef PERSONALMENU_H
#define PERSONALMENU_H

#include "../others/data.h"
#include "../others/define.h"
#include "../others/head.h"
#include "../others/IO.h"
#include "menu.h"
#include "../others/threadwork.h"
#include "groupmenu.h"
#include "filemenu.h"

#include <iostream>
using json = nlohmann::json;
using namespace std;

void personalmenuUI(void);
void showunreadnotice_client(int client_socket, string id, Queue<string> &RecvQue);
void logout_client(int client_socket, string username);
void addfriend_client(int client_socket, string username, Queue<string> &RecvQue);
void friendapplyedit_client(int client_socket, string id, Queue<string> &RecvQue);
string friendinfo_client(int client_socket, string id, Queue<string> &RecvQue, int fl);
void addblack_client(int client_socket, string id, Queue<string> &RecvQue);
void delfriend_client(int client_socket, string id, Queue<string> &RecvQue);
string blackfriendlist_client(int client_socket, string id, Queue<string> &RecvQue, int fl);
void blackfriendedit_client(int client_socket, string id, Queue<string> &RecvQue);
string historychat_client(int client_socket, string id, Queue<string> &RecvQue, int fl);
void chatfriend_client(int client_socket, string id, Queue<string> &RecvQue);
void personalinfo_client(int client_socket, string id, Queue<string> &RecvQue);
void group_client(int client_socket, string id, Queue<string> &RecvQue);
void file_client(int client_socket, string id, Queue<string> &RecvQue);

void messagemenu(int client_socket, string id, Queue<string> &RecvQue);

#endif