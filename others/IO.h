// 几个类，用于打包实现SendMsg（）和RecvMsg（）
#ifndef IO_H
#define IO_H

#include "head.h"
#include <iostream>

using namespace std;

class SendMsg
{
public:
    int writen(int fd, char *msg, int size);
    void SendMsg_client(int client_socket, const std::string &str);
    // void SendMsg_server(int client_socket, const std::string &str);
    void SendMsg_int(int client_socket, int state);
};

class RecvMsg
{
public:
    int readn(int fd, char *buf, int size);
    int RecvMsg_client(int client_socket, std::string &str);
    // void RecvMsg_server(int client_socket, std::string &str);
    int RecvMsg_int(int client_socket);
};

#endif
