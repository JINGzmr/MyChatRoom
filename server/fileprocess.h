// 服务器对客户端groupmenu里不同的选项进行不同的处理
#ifndef FILEROCESS_H
#define FILEPROCESS_H

#include "../others/data.h"
#include "../others/define.h"
#include "../others/IO.h"
#include "../others/redis.h"
#include <vector>

#include <iostream>
using json = nlohmann::json;
using namespace std;

// 向客户端发送文件列表
void sendfilelist_server(int fd, string buf);

// 向客户端发送文件
void sendfile_server(int fd, string buf);

// 接受来自客户端的文件
void recvfile_server(int fd, string buf);

#endif