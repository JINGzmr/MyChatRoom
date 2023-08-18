// 客户端发送文件选项界面，及与文件有关的事务
#ifndef FILEMENU_H
#define FILEMENU_H

#include "../others/data.h"
#include "../others/define.h"
#include "../others/head.h"
#include "../others/IO.h"
#include "menu.h"
#include "../others/threadwork.h"
#include "personalmenu.h"

#include <iostream>
using json = nlohmann::json;
using namespace std;

string friendinfo_client(int client_socket, string id, Queue<string> &RecvQue, int fl);

void filemenuUI(void);
void sendfile_client(int client_socket, string id, Queue<string> &RecvQue);
string recvfilelist_client(int client_socket, string id, Queue<string> &RecvQue);
void recvfile_client(int client_socket, string id, Queue<string> &RecvQue);

#endif