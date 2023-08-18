// 对客户端发来的各种json进行处理，并将事件加到工作队列，从线程池中取出工作线程进行相应的处理
#include "../others/threadpool.h"
#include "../others/IO.h"
#include "../others/data.h"
#include "../others/define.h"
#include "../others/redis.h"
#include "login.hpp"
#include "personalprocess.hpp"
#include "groupprocess.hpp"
#include "managegroupprocess.hpp"
#include "fileprocess.h"

#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

using json = nlohmann::json;
#define PORT 8080
#define ServerAddr "127.0.0.1"
const int MAX_CONN = 1024; // 最大连接数

void work(void *arg);

int main(int argc, char *argv[])
{
    // 默认值
    string serverAddr = ServerAddr;
    int port = PORT;

    // 解析命令行参数
    if (argc >= 2)
    {
        serverAddr = argv[1];
    }
    if (argc >= 3)
    {
        port = stoi(argv[2]);
    }

    // 创建线程池(24个线程)
    ThreadPool threadpool(24);

    // 创建监听的socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket error");
        return -1;
    }

    // 绑定本地ip和端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverAddr.c_str(), &addr.sin_addr.s_addr) <= 0)
    {
        perror("invalid address");
        return EXIT_FAILURE;
    }
    int ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (sockfd < 0)
    {
        perror("bind error");
        return -1;
    }

    // 监听客户端连接
    ret = listen(sockfd, 1024);
    if (ret < 0)
    {
        perror("listen error");
        return -1;
    }

    // 创建epoll实例
    int epld = epoll_create(10);
    if (epld < 0)
    {
        perror("epoll_create error");
        return -1;
    }

    // 将监听的socket加入epoll
    struct epoll_event ev;
    ev.events = EPOLLIN || EPOLLET; // 启用ET模式
    ev.data.fd = sockfd;
    ret = epoll_ctl(epld, EPOLL_CTL_ADD, sockfd, &ev);
    if (ret < 0)
    {
        perror("epoll_ctl error");
        return -1;
    }

    // 循环监听
    while (1)
    {
        struct epoll_event evs[MAX_CONN];
        int n = epoll_wait(epld, evs, MAX_CONN, -1);
        // if (n < 0)
        // {
        //     perror("epoll_wait error");
        //     break;
        // }

        for (int i = 0; i < n; i++) // 本次epoll一共监听到n个事件，即有n个fd，每次循环只针对单个fd（即一个客户端）
        {
            int fd = evs[i].data.fd;

            // 如果是监听的fd收到消息，那么则表示有客户端进行连接了
            if (fd == sockfd)
            {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_sockfd < 0)
                {
                    perror("accept error");
                    continue;
                }

                // 将客户端的socket加入epoll
                struct epoll_event ev_client;
                ev_client.events = EPOLLIN || EPOLLET; // 检测客户端有没有消息过来,且启用ET模式
                ev_client.data.fd = client_sockfd;

                // 设置非阻塞*************--->非阻塞的话，后面只要有出现recv的地方，就算没有数据可以读取，他也会不断的读数据，不断的读取失败。。。。。。
                int flag;
                flag = fcntl(client_sockfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(client_sockfd, F_SETFL, flag); // 设置非阻塞

                // 心跳检测（开启保活，1分钟内探测不到，断开连接）
                int keep_alive = 1;
                int keep_idle = 3;
                int keep_interval = 1;
                int keep_count = 30;
                if (setsockopt(client_sockfd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive)))
                {
                    perror("Error setsockopt(SO_KEEPALIVE) failed");
                    exit(1);
                }
                if (setsockopt(client_sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle)))
                {
                    perror("Error setsockopt(TCP_KEEPIDLE) failed");
                    exit(1);
                }
                if (setsockopt(client_sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval)))
                {
                    perror("Error setsockopt(TCP_KEEPINTVL) failed");
                    exit(1);
                }
                if (setsockopt(client_sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)))
                {
                    perror("Error setsockopt(TCP_KEEPCNT) failed");
                    exit(1);
                }

                // 将该客户端加入epoll树
                ret = epoll_ctl(epld, EPOLL_CTL_ADD, client_sockfd, &ev_client);
                if (ret < 0)
                {
                    perror("client epoll_ctl error");
                    break;
                }
                cout << client_addr.sin_addr.s_addr << "正在连接..." << endl;
            }

            else // 如果客户端有消息
            {
                // 从树上去除该套接字
                struct epoll_event temp;
                temp.data.fd = fd;
                temp.events = EPOLLIN || EPOLLET;
                epoll_ctl(epld, EPOLL_CTL_DEL, fd, &temp);

                // 把进入子线程，接下来的任务由子线程完成
                int arg[] = {fd, epld}; // 把fd和efd装到arg数组里，再作为参数传到work函数里，arg[0]=fd,arg[1]=epld
                Task task;
                task.function = work;
                task.arg = arg;
                threadpool.addTask(task);
            }
        }
    }

    // 关闭epoll实例及套接字
    close(epld);
    close(sockfd);
}

void work(void *arg)
{
    // 挂树的准备工作
    struct epoll_event temp;
    int *arr = (int *)arg;
    int fd = arr[0];
    int epld = arr[1];
    temp.data.fd = fd;
    temp.events = EPOLLIN; // 以上代码为后面 挂树 时有用，也是该函数传入参数的唯一作用

    // 服务端开的子线程负责接收客户端的消息
    // 客户端发送的json的string里包括几个部分：用户要执行的操作、用户名、发送对象（没有可以填空）、数据内容
    // 使用户发送的每个
    string recvJson_buf;
    RecvMsg recvmsg;
    int re = recvmsg.RecvMsg_client(fd, recvJson_buf); //***********(用第一版IO.cc时)第二次回到登录界面进行选择时，按下回车键，服务器会出现段错误，且停在这行过不去
    if (re == -1)                                      // 先判断客户端是否断开连接
    {
        // 更改在线情况
        Redis redis;
        redis.connect("127.0.0.1", 6379, "");
        if (redis.sismember("onlinelist", redis.gethash("usersocket_id", to_string(fd))) == 1)
            redis.sremvalue("onlinelist", redis.gethash("usersocket_id", to_string(fd)));

        close(fd);
        return;
    }
    json parsed_data = json::parse(recvJson_buf);
    // 不同结构体只要都有flag这个成员，那他们序列化产生的json字符串里的falg都能通过这行代码解析出来
    //  即：根据 键 来解析出对应的 值
    int flag_ = parsed_data["flag"];

    // 根据接收到的消息判断用户在登录界面的操作
    // if (flag_ != SENDFILE || flag_ != RECVFILE)
    {
        if (flag_ == LOGIN) // 登录
        {
            login_server(fd, recvJson_buf);
        }
        else if (flag_ == REGISTER) // 注册
        {
            register_server(fd, recvJson_buf);
        }
        else if (flag_ == SIGNOUT) // 注销
        {
            signout_server(fd, recvJson_buf);
        }
        else if (flag_ == FINDPASSWORD) // 找回密码
        {
            findpassword_server(fd, recvJson_buf);
        }
        else if (flag_ == OUT) // 退出
        {
            close(fd); // 和退出的客户端断开连接
            return;
        }
        else if (flag_ == SHOUNOTICE) // 展示离线消息
        {
            showunreadnotice_server(fd, recvJson_buf);
        }
        else if (flag_ == LOGOUT) // 退出登录
        {
            logout_server(fd, recvJson_buf);
        }
        else if (flag_ == ADDFRIEND)
        {
            addfriend_server(fd, recvJson_buf);
        }
        else if (flag_ == FRIENDAPPLYLIST)
        {
            friendapplylist_server(fd, recvJson_buf);
        }
        else if (flag_ == FRIENDAPPLYEDIT)
        {
            friendapplyedit_server(fd, recvJson_buf);
        }
        else if (flag_ == FRIENDINFO)
        {
            friendinfo_server(fd, recvJson_buf);
        }
        else if (flag_ == ADDBLACK)
        {
            addblack_server(fd, recvJson_buf);
        }
        else if (flag_ == DELFRIEND)
        {
            delfriend_server(fd, recvJson_buf);
        }
        else if (flag_ == BLACKFRIENDLIST)
        {
            blackfriendlist_server(fd, recvJson_buf);
        }
        else if (flag_ == BLACKFRIENDEDIT)
        {
            blackfriendedit_server(fd, recvJson_buf);
        }
        else if (flag_ == HISTORYCHAT)
        {
            historychat_server(fd, recvJson_buf);
        }
        else if (flag_ == CHATFRIEND)
        {
            chatfriend_server(fd, recvJson_buf);
        }
        else if (flag_ == PERSONALINFO)
        {
            personalinfo_server(fd, recvJson_buf);
        }
        else if (flag_ == CREATGROUP)
        {
            creatgroup_server(fd, recvJson_buf);
        }
        else if (flag_ == ADDGROUP)
        {
            addgroup_server(fd, recvJson_buf);
        }
        else if (flag_ == CHECKGROUP)
        {
            checkgroup_server(fd, recvJson_buf);
        }
        else if (flag_ == OUTGROUP)
        {
            outgroup_server(fd, recvJson_buf);
        }
        else if (flag_ == CHECKGROUPNUM)
        {
            checkgroupnum_server(fd, recvJson_buf);
        }
        else if (flag_ == ADDADMIN)
        {
            addmin_server(fd, recvJson_buf);
        }
        else if (flag_ == DELADMIN)
        {
            deladmin_server(fd, recvJson_buf);
        }
        else if (flag_ == CHECKAPPLYLIST)
        {
            checkapplylist_server(fd, recvJson_buf);
        }
        else if (flag_ == AGREEAPPLY)
        {
            agreeapply_server(fd, recvJson_buf);
        }
        else if (flag_ == DELGROUPNUM)
        {
            delgroupnum_server(fd, recvJson_buf);
        }
        else if (flag_ == DELGROUP)
        {
            delgroup_server(fd, recvJson_buf);
        }
        else if (flag_ == HISTORYGROUPCHAT)
        {
            historygroupchat_server(fd, recvJson_buf);
        }
        else if (flag_ == GROUPCHAT)
        {
            groupchat_server(fd, recvJson_buf);
        }
        else if (flag_ == SENDFILE) // 客户端发送文件，服务端调用接收文件的函数
        {
            recvfile_server(fd, recvJson_buf);
        }
        else if (flag_ == RECVFILELIST)
        {
            sendfilelist_server(fd, recvJson_buf);
        }
        else if (flag_ = RECVFILE)
        {
            sendfile_server(fd, recvJson_buf);
        }
    }

    epoll_ctl(epld, EPOLL_CTL_ADD, fd, &temp);
    return;
}