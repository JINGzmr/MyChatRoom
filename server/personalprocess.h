// 服务器对客户端personalmenu里不同的选项进行不同的处理
#ifndef PERSONALPROCESS_H
#define PERSONALPROCESS_H

#include "../others/data.h"
#include "../others/define.h"
#include "../others/IO.h"
#include "../others/redis.h"
#include <vector>

#include <iostream>
using namespace std;

// 展示未通知消息
void showunreadnotice_server(int fd, string buf);

// 退出登录
void logout_server(int fd, string buf);

// 加好友
void addfriend_server(int fd, string buf);

// 好友申请----->在函数里用到了recv，如果是非阻塞的话，recv会显示接受失败，然后就和那个客户端断开了连接
// 查看申请列表
void friendapplylist_server(int fd, string buf);

// 编辑好友申请
void friendapplyedit_server(int fd, string buf);

// 好友信息
void friendinfo_server(int fd, string buf);

// 屏蔽好友
void addblack_server(int fd, string buf);

// 删除好友
void delfriend_server(int fd, string buf);

// 查看屏蔽好友列表
void blackfriendlist_server(int fd, string buf);

// 编辑屏蔽好友
void blackfriendedit_server(int fd, string buf);

// 好友聊天历史记录
void historychat_server(int fd, string buf);

// 与好友聊天
void chatfriend_server(int fd, string buf);

// 个人信息
void personalinfo_server(int fd, string buf);

#endif