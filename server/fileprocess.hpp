// 服务器对客户端groupmenu里不同的选项进行不同的处理
#ifndef FILEROCESS_HPP
#define FILEPROCESS_HPP

#include "../others/data.h"
#include "../others/define.h"
#include "../others/IO.h"
#include "../others/redis.hpp"
// #include <vector>

#include <iostream>
using json = nlohmann::json;
using namespace std;

// 向客户端发送文件
void sendfile_server(int fd, string buf)
{
}

// 接受来自客户端的文件
void recvfile_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct File file;
    file.oppoid = parsed_data["oppoid"];
    file.id = parsed_data["id"];
    file.filename = parsed_data["filename"];
    file.filesize = parsed_data["filesize"];
    printf("--- %s 用户将要向 %s 用户发送文件 ---\n", file.id.c_str(), file.oppoid.c_str());
    cout << file.filename << endl;

    Redis redis;
    redis.connect();

    file.filepath = "/home/zhangminrui/桌面/chatroom/file/"+file.filename;

    // 创建文件
    FILE *fp = fopen(file.filepath.c_str(), "wb");
    if (fp == NULL)
    {
        perror("fopen fail");
        return;
    }

    // 把数据写入文件
    RecvMsg recvmsg;
    int len; // 返回接收到的字节数
    char buffer[BUFSIZ];
    off_t sum = 0;
    while (file.filesize > 0)
    {
        if (sizeof(buf) < file.filesize)
        {
            len = recvmsg.readn(fd, buffer, sizeof(buf));
        }
        else
        {
            len = recvmsg.readn(fd, buffer, file.filesize);
        }
        if (len < 0)
        {
            continue;
        }

        file.filesize -= len;
        sum += len;
        fwrite(buffer, len, 1, fp); // 写到文件里
    }

    redis.saddvalue(file.id+"to"+file.oppoid+":file" , file.filename);
    fclose(fp);

    if (redis.sismember("onlinelist", file.oppoid) == 1) // 在线列表里有对方
    {
        cout << "对方在线" << endl;
        file.msg = redis.gethash("id_name", file.id) + "向你发送文件";
        file.state = SUCCESS;
        file.type = NOTICE;
    }
    else // 对方不在线：加入数据库，等用户上线时提醒
    {
        cout << "对方不在线" << endl;
        file.msg = redis.gethash("id_name", file.id) + "向你发送文件";
        file.state = SUCCESS;
        file.type = NORMAL;

        // 加入到对方的未读通知消息队列里
        redis.saddvalue(file.oppoid + ":unreadnotice", file.msg);
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", file.type},
        {"state", file.state},
        {"msg", file.msg},
        {"flag", 0},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    if (file.type == NORMAL)
    {
        sendmsg.SendMsg_client(fd, json_string);
    }
    else if (file.type == NOTICE) // 如果是通知消息，那就把这条消息发给对方（所以下面要根据对方的id获得对方的socket）
    {
        sendmsg.SendMsg_client(stoi(redis.gethash("usersocket", file.oppoid)), json_string);

        // 改成正常的类型后给本用户的客户端发回去，不然客户端接不到事件的处理进度
        json_["type"] = NORMAL;
        json_string = json_.dump();
        sendmsg.SendMsg_client(fd, json_string);
    }
    return;
}

#endif