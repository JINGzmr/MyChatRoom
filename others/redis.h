#ifndef REDIS_H
#define REDIS_H

// 无重复，按字典序：sadd
// 可重复，按存储时间：lpush（存聊天记录刚刚好）

#include "../others/define.h"

#include <hiredis/hiredis.h>
#include <string>
#include <cstring>
#include <iostream>
using namespace std;

// 数据库有多个地方存数据
// info的键：对应的是存所有用户的UID
// peopleinfo的键：对应的是哈希表，字段存的是用户的UID，值存序列化好的用户个人信息


// 哈希表中：
// 用户信息userinfo作为键，字段是id，值是序列化好的用户个人信息
// id与昵称对应 id_name为键，字段id，值：昵称
// 昵称与id对应 name_id为键，字段昵称，值：id
// id与套接字对应 usersocket为键，字段为id，值：socket
// 套接字与id对应 usersocket_id为键，字段为socket，值：id

// id与群名对应 groupname_id,name,id
// 群名与id对应 groupid_name,id,name


// 普通表中：
// username用来存放用户名,onlinelist：在线用户列表
// id+:friends：id对应用户的好友
// id+:friendsapply：id对应用户的好友申请
// id+:bfriends：id对应的拉黑用户
// id+:unreadnotice：id对应用户的未读通知

// groupname 存放群聊的名称
// id+:group,groupid：id对应用户加入的群聊
// groupid+:num,id：群成员
// groupid+:admin,id：群管理（包括群主）
// groupid+:groupapply,id：群聊申请表--->服务器在准备取该申请表前，先检查该群id在不在
// id+:mycreatgroup,groupid：id对应用户创建的群聊
// id+:myadmingroup,groupid：id对应用户管理的群聊(不包括群主)

// id1+to+id2+:file：1给2发的文件，在用户2中可以查看


// list表中：
// 历史消息：小的id+大的id+:historychat为键，消息的结构体为值
// 群历史消息：groupid+:historygroupchat为键，消息的结构体为值

class Redis
{
public:
    Redis();
    ~Redis();
    int connect(const string &addr, int port, const string &pwd); // 连接redis数据库：addr：IP地址，port：端口号，pwd：密码(默认为空)
    int disConnect();                                             // 断开连接

    int hsetValue(const string &key, const string &field, const string &value); // 插入哈希表
    int hashexists(const string &key, const string &field);                     // 查看是否存在，存在返回1，不存在返回0
    string gethash(const string &key, const string &field);                     // 获取对应的hash_value
    int hashdel(const string &key, const string &field);                        // 从哈希表删除指定的元素

    int saddvalue(const string &key, const string &value); // 插入到集合
    int sismember(const string &key, const string &value); // 查看数据是否存在
    int sremvalue(const string &key, const string &value); // 将数据从set中移出
    int scard(const string &key);                          // set中元素的个数
    redisReply **smembers(const string &key);              // 取出成员

    int lpush(const string &key, const string &value);
    int llen(const string &key);
    redisReply **lrange(const string &key);                     //返回所有消息
    int ltrim(const string &key);                               //删除链表中的所有元素


private:
    string m_addr;        // IP地址
    int m_port;           // 端口号
    string m_pwd;         // 密码
    redisContext *pm_rct; // redis结构体--->在connect函数里被赋值
    redisReply *pm_rr;    // 返回结构体
};


#endif