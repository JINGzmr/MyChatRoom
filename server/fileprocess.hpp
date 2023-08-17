// 服务器对客户端groupmenu里不同的选项进行不同的处理
#ifndef FILEROCESS_HPP
#define FILEPROCESS_HPP

#include "../others/data.h"
#include "../others/define.h"
#include "../others/IO.h"
#include "../others/redis.hpp"
#include <vector>

#include <iostream>
using json = nlohmann::json;
using namespace std;

// 向客户端发送文件列表
void sendfilelist_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct File file;
    file.id = parsed_data["id"];
    file.opponame = parsed_data["opponame"];
    printf("--- %s 用户查看 %s 发来的文件 ---\n", file.id.c_str(), file.opponame.c_str());

    Redis redis;
    redis.connect();

    vector<string> files_Vector; // 放文件的容器

    if (redis.sismember("username", file.opponame) != 1)
    {
        cout << "账号不存在" << endl;
        file.state = USERNAMEUNEXIST;
    }
    else if (redis.sismember(file.id + ":friends", redis.gethash("name_id", file.opponame)) != 1) // 对方不是你的好友
    {
        cout << "对方不是你好友" << endl;
        file.state = FAIL;
    }
    else
    {
        file.oppoid = redis.gethash("name_id", file.opponame); // 用户注销完会从该哈希表中去除，所以用name找到的id也是独一无二的

        int len = redis.scard(file.oppoid + "to" + file.id + ":file");
        redisReply **arry = redis.smembers(file.oppoid + "to" + file.id + ":file");

        // 把数据从数据库转移到容器里
        for (int i = 0; i < len; i++)
        {
            // 得到文件json的字符串
            string filename = arry[i]->str;

            files_Vector.push_back(filename);

            freeReplyObject(arry[i]);
        }
        file.state = SUCCESS;
    }
    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"vector", files_Vector},
        {"state", file.state},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 向客户端发送文件
void sendfile_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct File file;
    file.id = parsed_data["id"];
    file.opponame = parsed_data["opponame"];
    file.filename = parsed_data["filename"];
    file.filepath = parsed_data["filepath"];
    printf("--- %s 用户接收 %s 发来的文件 ---\n", file.id.c_str(), file.opponame.c_str());

    Redis redis;
    redis.connect();

    string file_path = "/home/zhangminrui/桌面/chatroom/file/" + file.filename;

    // 打开文件
    int fp = open(file_path.c_str(),  O_RDONLY, S_IRUSR | S_IWUSR | O_LARGEFILE);
    if (fp == -1)
    {
        perror("open");
        return;
    }
    off_t len = 0; // 每次发送的字节数
    off_t sum = 0; // 累计发送的字节数
    off_t offset = 0;

    // 获取文件大小（字节数）
    struct stat info;
    stat(file_path.c_str(), &info);
    file.filesize = info.st_size;
    cout << "filesize:" << file.filesize << endl;

    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", READYSEND},
        {"id", file.id},
        {"filepath", file.filepath},
        {"filesize", file.filesize},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
    // sleep(10);

    // 向客户端发送文件
    while (1)
    {
        len = sendfile(fd, fp, &offset, file.filesize - offset);
        cout << "len:" << len << endl;
        system("sync"); // 文件系统缓冲区中的数据同步到磁盘
        if (len == 0 && sum == file.filesize)
        {
            cout << "文件发送完毕！" << endl;
            break;
        }
        else if (len > 0)
        {
            sum += len;
            cout << "sum:" << sum << endl;
            // 设置输出格式，保留2位小数
            cout << fixed << setprecision(2);
            cout << static_cast<double>(sum) / file.filesize * 100 << "%"
                 << "..." << endl;
        }
        else
        {
            perror("sendfile");
        }
    }
    // file.oppoid = redis.gethash("name_id", file.opponame);
    // redis.sremvalue(file.id + "to" + file.oppoid + ":file", file.filename);
    close(fp);
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

    // file.filepath = "/home/zhangminrui/桌面/chatroom/file/" + file.filename;
    file.filepath = "/home/" + file.filename;

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

    // 把文件名放入数据库
    redis.saddvalue(file.id + "to" + file.oppoid + ":file", file.filename);
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