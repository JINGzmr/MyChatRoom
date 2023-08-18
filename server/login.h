// 服务端处理客户端在登录界面输入的注册、登入、注销
#ifndef LOGIN_H
#define LOGIN_H

#include <iostream>
using namespace std;

// 登录
void login_server(int fd, string buf);

// 注册
void register_server(int fd, string buf);

// 注销
void signout_server(int fd, string buf);

// 找回密码
void findpassword_server(int fd, string buf);

#endif