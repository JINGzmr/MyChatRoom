// 存头文件的，但可能到后期会有些问题
// 比如，a.hpp里调用了b.hpp的内容，那么b.hpp就要放在a.hpp的前面
#ifndef HEAD_H
#define HEAD_H

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> //send,recv
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <regex>
#include <cstdlib> //system("clear");
#include <thread> 
#include <termios.h> //屏蔽ctrl+d
// #include <ncurses.h> //关联f11，表示退出聊天

#include <cstring>     //memset
#include <arpa/inet.h> //htonl
// #include <jsoncpp/json/json.h>

using namespace std;
using json = nlohmann::json;

#endif