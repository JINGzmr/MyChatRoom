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
// #include <libgen.h> // 由文件路径获取文件名的basename()函数
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <iomanip> // 包含iomanip头文件用于设置输出格式

#include <cstring>     //memset
#include <arpa/inet.h> //htonl
// #include <jsoncpp/json/json.h>

using namespace std;
using json = nlohmann::json;

#endif