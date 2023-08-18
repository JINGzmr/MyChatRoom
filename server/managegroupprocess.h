// 服务器对客户端managegroupmenu里不同的选项进行不同的处理
#ifndef MANAGEGROUPPROCESS_H
#define MANAGEGROUPPROCESS_H

#include <iostream>
using namespace std;

// 添加管理员（通知被添加的管理员）
void addmin_server(int fd, string buf);

// 删除管理员（通知被删除的管理员）
void deladmin_server(int fd, string buf);

// 查看申请加群列表
void checkapplylist_server(int fd, string buf);

// 同意加群申请
void agreeapply_server(int fd, string buf);

// 删除群成员（通知被删除的成员）
void delgroupnum_server(int fd, string buf);

// 解散群组(不通知)
void delgroup_server(int fd, string buf);

#endif