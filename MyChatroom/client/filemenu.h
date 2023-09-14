// 客户端发送文件选项界面，及与文件有关的事务
#ifndef FILEMENU_H
#define FILEMENU_H

#include "../others/queue.hpp"

#include <iostream>
using namespace std;

void filemenuUI(void);
void sendfile_client(int client_socket, string id, Queue<string> &RecvQue);
string recvfilelist_client(int client_socket, string id, Queue<string> &RecvQue);
void recvfile_client(int client_socket, string id, Queue<string> &RecvQue);

#endif