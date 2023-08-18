#ifndef MENU_H
#define MENU_H

// 菜单界面，对接的是客户端
// 几个函数，客户端在不同的菜单栏选择不同的选项，进入不同的模块
// 在不同的模块下对用户输入的各种数据进行json序列化和IO的SendMsg
// 并且接收RecvMsg来自服务端发送回来的数据，进行反序列化，打印在页面上
#include "../others/head.h"
#include "../others/data.h"
#include "../others/define.h"
#include "../others/IO.h"
#include "personalmenu.h"
#include "../others/threadwork.h"
#include "../others/queue.hpp"

#include <iostream>
using json = nlohmann::json;
using namespace std;

// menu调用了下面的函数，因这几个函数是在同一个文件里的，所以得把声明写在menu的前面
// 否者会显示调用失败
void login_client(int client_socket);
void register_client(int client_socket);
void signout_client(int client_socket);
void out_client(int client_socket);
void findpassword_client(int client_socket);
void menu(int client_socket);



#endif