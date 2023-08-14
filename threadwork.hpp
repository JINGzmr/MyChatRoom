// 客户端的工作线程，客户端所有的recv都在这里接收
// 工作线程根据recv的标头来判断是 返回的消息 还是 通知事件
// 如果是返回的消息，则将这个消息加入到消息队列里，客户端的子线程从消息队列里取消息
// 如果是通知消息，判断是否是聊天消息，私聊还是群聊，用户是否在聊天室里

#ifndef THREADWORK_H
#define THREADWORK_H

#include "head.h"
#include "queue.hpp"
#include "define.h"
#include "data.h"

// 根据type来决定是否将受到的recvjson_buf放入消息队列里
// type：普通normal、通知notice
// 每个收到的buf都有type、flag

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
            return;

        if (type == NOTICE)
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
            cout << "\033[30;1m（ 一条消息放入消息队列 ）\033[0m" << endl; // 深灰色
            RecvQue->add(recvJson_buf);
        }
        // cout << "3" << endl;
    }
}

#endif
