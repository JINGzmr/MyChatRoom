// 客户端发送文件选项界面，及与文件有关的事务
#ifndef FILEMENU_HPP
#define FILEMENU_HPP

#include "../others/data.h"
#include "../others/define.h"
#include "../others/head.h"
#include "../others/IO.h"
#include "menu.hpp"
#include "../others/threadwork.hpp"
#include "personalmenu.hpp"

#include <iostream>
using json = nlohmann::json;
using namespace std;

string friendinfo_client(int client_socket, string id, Queue<string> &RecvQue, int fl);

void filemenuUI(void)
{
    cout << "——————————————————————————————————————————————————" << endl;
    cout << "--------------------  文件传输  -------------------" << endl;
    cout << "——————————————————————————————————————————————————" << endl;
    cout << "                      1.发送文件                  " << endl;
    cout << "                      2.接收文件                   " << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "                      3.返回上一级                 " << endl;
    cout << "---------------------------------------------------" << endl;
    cout << "                      0.刷新页面                   " << endl;
    cout << "———————————————————————————————————————————————————" << endl;
}

// 发送文件
// 先要获取要发送给对方的id，文件名，将这些信息发送给服务器，让服务器将信息存到redis里（同时将这个消息通知给对方）
// 接着服务器专门开一个线程负责接收来自客户端上传的文件（文件大，得sendfile和recv好多次，在同一个线程里完成，每次记录文件的偏移量）
// 并把接收到的文件先存到服务器所属的本机上
void sendfile_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 先打印出好友信息
    string re = friendinfo_client(client_socket, id, RecvQue, 0);
    if (re != "fail")
    {
        struct stat info;
        File file;
        cout << "请输入要发送文件的好友id：" << endl;
        file.oppoid = getInputWithoutCtrlD();
        file.id = id;
        while (1)
        {
            cout << "请输入文件路径名：" << endl;
            file.filepath = getInputWithoutCtrlD();

            if (stat(file.filepath.c_str(), &info) == -1)
            {
                printf("路径错误！请重新输入\n");
            }
            else
            {
                break;
            }
        }
        // file.filename = basename(file.filepath.c_str());
        filesystem::path path(file.filepath);
        file.filename = path.filename().string();
        file.flag = SENDFILE;

        // 打开文件
        int fp = open(file.filepath.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR | O_LARGEFILE);
        if (fp == -1)
        {
            perror("open");
            return;
        }
        off_t len = 0; // 每次发送的字节数
        off_t sum = 0; // 累计发送的字节数

        // 获取文件大小（字节数）
        file.filesize = info.st_size;

        // 发送数据(包括文件大小)
        nlohmann::json sendJson_client = {
            {"oppoid", file.oppoid},
            {"id", file.id},
            {"filename", file.filename},
            {"filesize", file.filesize},
            {"flag", file.flag},
        };
        string sendJson_client_string = sendJson_client.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // 向服务器发送文件
        while (1)
        {
            len = sendfile(client_socket, fp, NULL, file.filesize);
            // cout << "len:" << len << endl;
            system("sync"); // 文件系统缓冲区中的数据同步到磁盘
            if (len == 0 && sum == file.filesize)
            {
                cout << "文件发送完毕！" << endl;
                break;
            }
            else if (len > 0)
            {
                sum += len;
                // cout << "sum:" << sum << endl;
                // cout << "filesize:" << file.filesize << endl;

                cout << sum / file.filesize * 100 << "%"
                     << "..." << endl;
            }
            else
            {
                perror("sendfile");
            }
        }

        close(fp);

        // 从消息队列里取消息
        string buf = RecvQue.remove();
        json parsed_data = json::parse(buf);
        int state_ = parsed_data["state"];
        if (state_ == SUCCESS)
        {
            cout << "文件上传成功！" << endl;
        }
        else
        {
            cout << "文件上传失败！" << endl;
        }
    }

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
    return;
}

// 接收文件
// 先选择要接受来自谁的文件，文件名是什么
// 接着服务器也是开一个线程负责将本机上的文件多次发送给接收文件的客户端，直到发送完毕
string recvfilelist_client(int client_socket, string id, Queue<string> &RecvQue)
{

    // 先打印出好友信息
    string re = friendinfo_client(client_socket, id, RecvQue, 0);
    string str;

    if (re != "fail")
    {
        File file;
        cout << "输入好友昵称：" << endl;
        file.opponame = getInputWithoutCtrlD();
        file.id = id;

        // 发送数据
        nlohmann::json sendJson_client = {
            {"id", file.id},
            {"opponame", file.opponame},
            {"flag", RECVFILELIST},
        };
        string sendJson_client_string = sendJson_client.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // 从消息队列里取消息
        string buf = RecvQue.remove();
        json parsed_data = json::parse(buf);
        vector<string> files_Vector = parsed_data["vector"];
        int state_ = parsed_data["state"];

        if (state_ == FAIL)
        {
            cout << "对方不是你的好友！" << endl;
            str = "fail";
        }
        else if (state_ == USERNAMEUNEXIST)
        {
            cout << "账号不存在！" << endl;
            str = "fail";
        }
        else if (files_Vector.empty())
        {
            cout << "———————————————暂无文件——————————————" << endl;
            str = "fail";
        }
        else
        {
            // 循环打印输出
            cout << "————————————————————————————————————————————————" << endl;
            for (int i = files_Vector.size() - 1; i >= 0; i--)
            {
                cout << files_Vector[i] << endl;
            }
            cout << "————————————————以上为该好友向你发送的全部文件———————————————————" << endl;
            str = file.opponame;
        }
    }

    return str;
}

void recvfile_client(int client_socket, string id, Queue<string> &RecvQue)
{
    File file;
    file.opponame = recvfilelist_client(client_socket, id, RecvQue);
    if (file.opponame != "fail")
    {
        // 选择要接收的文件名，准备开始接受文件
        cout << "输入要接收的文件名：" << endl;
        file.filename = getInputWithoutCtrlD();
        cout << "输入文件的保存路径：" << endl;
        file.filepath = getInputWithoutCtrlD() + "/" + file.filename;
        file.id = id;

        // 发送数据
        nlohmann::json sendJson_client = {
            {"filename", file.filename},
            {"id", file.id},
            {"opponame", file.opponame},
            {"filepath", file.filepath},
            {"flag", RECVFILE},
        };
        string sendJson_client_string = sendJson_client.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // // 创建文件
        // FILE *fp = fopen(file.filepath.c_str(), "wb");
        // if (fp == NULL)
        // {
        //     perror("fopen fail");
        //     return;
        // }

        // // 把数据写入文件
        // RecvMsg recvmsg;
        // int len; // 返回接收到的字节数
        // char buffer[BUFSIZ];
        // off_t sum = 0;
        // while (file.filesize > 0)
        // {
        //     if (sizeof(buffer) < file.filesize)
        //     {
        //         len = recvmsg.readn(client_socket, buffer, sizeof(buffer));
        //     }
        //     else
        //     {
        //         len = recvmsg.readn(client_socket, buffer, file.filesize);
        //     }
        //     if (len < 0)
        //     {
        //         continue;
        //     }

        //     file.filesize -= len;
        //     sum += len;
        //     fwrite(buffer, len, 1, fp); // 写到文件里
        // }
        // fclose(fp);
        string buf = RecvQue.remove();
        if (buf == "ok")
        {
            cout << "文件下载成功！" << endl;
        }
        else
        {
            cout << "文件下载失败！" << endl;
        }

        // thread RecvThread = thread(recvfunc, client_socket, id, &RecvQue); // 工作线程启动
    }

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
}
#endif