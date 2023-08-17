// 客户端的工作线程，客户端所有的recv都在这里接收
// 工作线程根据recv的标头来判断是 返回的消息 还是 通知事件
// 如果是返回的消息，则将这个消息加入到消息队列里，客户端的子线程从消息队列里取消息
// 如果是通知消息，判断是否是聊天消息，私聊还是群聊，用户是否在聊天室里

#ifndef THREADWORK_H
#define THREADWORK_H

#include "head.h"
#include "queue.hpp"
#include "../others/define.h"
#include "../others/data.h"

// 根据type来决定是否将受到的recvjson_buf放入消息队列里
// type：普通normal、通知notice
// 每个收到的buf都有type、flag

// 当文文件传输时，不应该进入工作线程
void recvfunc(int fd, string id, Queue<string> *RecvQue)
{
    while (1)
    {
        // cout << "-1" << endl;

        // 接收服务器返回的数据
        string recvJson_buf = "";
        RecvMsg recvmsg;
        // cout << "0" << endl;

        while (recvJson_buf.empty()) // 不为空时，表明本次接收到了消息，退出循环
        {
            recvmsg.RecvMsg_client(fd, recvJson_buf);
        }
        // cout << "1" << endl;
        // 反序列化
        json parsed_data = json::parse(recvJson_buf);
        int type = parsed_data["type"];
        int flag = parsed_data["flag"];
        // cout << "2" << endl;

        if (flag == LOGOUT)
            break;

        if (flag == READYSEND)
        {
            // cout << "111"<< endl;
            struct File file;
            file.id = parsed_data["id"];
            file.filepath = parsed_data["filepath"];
            file.filesize = parsed_data["filesize"];

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
                if (sizeof(buffer) < file.filesize)
                {
                    len = recvmsg.readn(fd, buffer, sizeof(buffer));
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
            fclose(fp);
            RecvQue->add("ok");
        }
        else if (type == NOTICE)
        {
            if (flag == PRIVATE)
            {
                // string id = parsed_data["id"];     // 发送信息方的id
                string name = parsed_data["name"]; // 发送信息方的名字
                if (name != chatname)              // 对方不在聊天窗口，chatname是当用户进入聊天窗口时更改
                {
                    cout << "\033[90m                            " << name << "发来新消息\033[0m" << endl; // 灰色
                }
                else // 对方在聊天窗口，则直接打印
                {
                    string msg = parsed_data["msg"];
                    cout << "\033[32m" << name << ": " << msg << "\033[0m" << endl; // 深绿色
                }
            }
            else if (flag == GROUP)
            {
                string groupid = parsed_data["groupid"];     // 群id
                string groupname = parsed_data["groupname"]; // 群名
                string name = parsed_data["name"];           // 发送消息的人的名字

                if (groupid != chatgroup) // 对方不在聊天窗口，chatgroup是当用户进入聊天窗口时更改
                {
                    cout << "\033[90m                            " << groupname << " 群有新消息\033[0m" << endl; // 灰色
                }
                else // 对方在聊天窗口，则直接打印
                {
                    string msg = parsed_data["msg"];
                    cout << "\033[32m" << name << ": " << msg << "\033[0m" << endl; // 深绿色
                }
            }
            else
            {
                string msg = parsed_data["msg"];
                cout << "\033[90m                              " << msg << "\033[0m" << endl; // 灰色
            }
        }
        else if (type == NORMAL)
        {
            // cout << "\033[30;1m（ 一条消息放入消息队列 ）\033[0m" << endl; // 深灰色
            RecvQue->add(recvJson_buf);
        }
        // cout << "3" << endl;
    }
}

#endif
