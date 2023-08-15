#include "menu.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <csignal>

#define ServerAddr "127.0.0.1"
#define PORT 8080

int main(int argc, char *argv[])
{
    // 忽略SIGINT和SIGTSTP信号
    signal(SIGINT, SIG_IGN); // 忽略Ctrl+C
    // signal(SIGTSTP, SIG_IGN); // 忽略Ctrl+Z

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

    // 创建连接对象
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("socket creation failed");
        return EXIT_FAILURE;
    }

    // 客户端连接的IP和端口号
    sockaddr_in server_address{};
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, serverAddr.c_str(), &server_address.sin_addr) <= 0)
    {
        perror("请输入正确的ip地址及端口号！");
        return EXIT_FAILURE;
    }

    // 连接服务器
    int connect_result = connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    if (connect_result == -1)
    {
        perror("connection failed");
        return EXIT_FAILURE;
    }
    std::cout << connect_result << std::endl;
    std::cout << "欢迎来到私人聊天室" << std::endl;

    // 进入登录界面
    menu(client_socket);

    // 关闭套接字
    close(client_socket);
}
