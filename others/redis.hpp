#ifndef REDIS_HPP
#define REDIS_HPP

// 无重复，按字典序：sadd
// 可重复，按存储时间：lpush（存聊天记录刚刚好）

#include "../others/define.h"

#include <hiredis/hiredis.h>
#include <string>
#include <cstring>
#include <iostream>
using namespace std;

// **********
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
    int setValue(const string &key, const string &value);         // 添加或修改键值对，成功返回0，失败<0
    string getValue(const string &key);                           // 获取键对应的值，成功返回0，失败<0
    int delKey(const string &key);                                // 删除键，成功返回影响的行数，失败<0

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
    redisReply **lrange(const string &key, string a, string b); //返回指定的消息记录
    int ltrim(const string &key);                               //删除链表中的所有元素


private:
    string m_addr;        // IP地址
    int m_port;           // 端口号
    string m_pwd;         // 密码
    redisContext *pm_rct; // redis结构体--->在connect函数里被赋值
    redisReply *pm_rr;    // 返回结构体
};

Redis::Redis()
{
    m_addr = "";
    m_port = 0;
    m_pwd = "";
    pm_rct = NULL;
    pm_rr = NULL;
}
Redis::~Redis()
{
    disConnect();
    pm_rct = NULL;
    pm_rr = NULL;
}

// 连接数据库（参数：redis服务器的IP地址、端口号、密码）
//  ----这段代码的目的是连接到Redis数据库。它接受服务器的IP地址、端口号和密码作为参数，并将连接结果存储在类的成员变量中
int Redis::connect(const string &addr = "127.0.0.1", int port = 6379, const string &pwd = "")
{
    // 将传入参数存到类的成员变量里
    m_addr = addr;
    m_port = port;
    m_pwd = pwd;

    // 调用redisConnect函数来实际连接到Redis服务器
    // 接受服务器的IP地址和端口号作为参数，并返回一个指向redisContext结构体的指针
    pm_rct = redisConnect(m_addr.c_str(), m_port);
    if (pm_rct->err) // 失败返回-1
    {
        return FAIL;
    }
    return SUCCESS; // 成功返回0
}

// 断开链接
int Redis::disConnect()
{
    // freeReplyObject(pm_rr);
    redisFree(pm_rct);
    return 1;
}
int Redis::setValue(const string &key, const string &value)
{
    string cmd = "set  " + key + "  " + value;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->type;
    freeReplyObject(pm_rr);
    return p;
}
string Redis::getValue(const string &key) // 获取键对应的值，成功返回0，失败<0
{
    string cmd = "get  " + key;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    string p = pm_rr->str;
    freeReplyObject(pm_rr);
    return p;
}
int Redis::delKey(const string &key)
{
    string cmd = "del  " + key;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->type;
    freeReplyObject(pm_rr);
    return p;
}

// 向数据库中的哈希表插入数据（参数：哈希表的键、字段、值）
// ------这段代码的作用是通过调用Redis命令将指定的字段和值插入到指定的哈希表中，并返回执行结果的类型。
int Redis::hsetValue(const string &key, const string &field, const string &value) // 插入哈希表
{
    // 构建了一个Redis命令字符串cmd，该命令用于将指定的field和value插入到指定的key对应的哈希表中
    string cmd = "hset  " + key + " " + field + " " + value;

    // 调用redisCommand函数执行Redis命令
    // redisCommand函数接受两个参数：一个是Redis连接对象，另一个是要执行的命令字符串
    // pm_rct表示Redis连接对象，cmd.c_str()将命令字符串转换为C风格字符串
    // 执行命令后，将返回的结果赋值给pm_rr，这是一个指向redisReply类型的指针
    // redisReply结构体用于保存Redis命令的执行结果
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());

    // 通过访问pm_rr->type来获取执行结果的类型
    // 这里的type表示执行结果的类型
    int p = pm_rr->type;

    // 调用freeReplyObject函数释放pm_rr指针指向的内存
    freeReplyObject(pm_rr);

    // 返回变量p，它保存了执行结果的类型
    return p;
}

int Redis::hashexists(const string &key, const string &field) // 查看是否存在，存在返回1，不存在返回0
{
    string cmd = "hexists  " + key + "  " + field;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->integer;
    freeReplyObject(pm_rr);
    return p;
}
string Redis::gethash(const string &key, const string &field) // 获取对应的hash_value
{
    string cmd = "hget  " + key + "  " + field;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    string p = pm_rr->str;
    freeReplyObject(pm_rr);
    return p;
}
int Redis::hashdel(const string &key, const string &field) // 从哈希表删除指定的元素，成功返回3
{
    string cmd = "hdel  " + key + "  " + field;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->type;
    freeReplyObject(pm_rr);
    return p;
}

/*----------------------------------------*/
int Redis::saddvalue(const string &key, const string &value) // 插入到集合
{
    string cmd = "sadd  " + key + "  " + value;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->type;
    freeReplyObject(pm_rr);
    return p;
}
int Redis::sismember(const string &key, const string &value) // 查看数据是否存在 ,存在返回1
{
    string cmd = "sismember  " + key + "  " + value;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->integer;
    freeReplyObject(pm_rr);
    return p;
}
int Redis::sremvalue(const string &key, const string &value) // 将数据从set中移出 ,成功返回3
{
    string cmd = "srem  " + key + "  " + value;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->type;
    freeReplyObject(pm_rr);
    return p;
}
int Redis::scard(const string &key) // set中元素的个数
{
    string cmd = "scard  " + key;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->integer;
    freeReplyObject(pm_rr);
    return p;
}

redisReply **Redis::smembers(const string &key) //  取出成员
{
    string cmd = "smembers  " + key;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    return pm_rr->element;
}

/*----------------------------------------*/
int Redis::lpush(const string &key, const string &value)
{
    string cmd = "lpush  " + key + " " + value;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->type;
    freeReplyObject(pm_rr);
    return p;
}
int Redis::llen(const string &key)
{
    string cmd = "llen  " + key;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->integer;
    freeReplyObject(pm_rr);
    return p;
}

redisReply **Redis::lrange(const string &key) //返回所有消息
{
    string cmd = "lrange  " + key + "  0" + "  -1";
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    return pm_rr->element;
}

redisReply **Redis::lrange(const string &key, string a, string b) //返回指定的消息记录
{
    string cmd = "lrange  " + key + "  " + a + "  " + b;
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    return pm_rr->element;
}

int Redis::ltrim(const string &key) //删除链表中的所有元素
{
    string cmd = "ltrim  " + key + " 1 " + " 0 ";
    pm_rr = (redisReply *)redisCommand(pm_rct, cmd.c_str());
    int p = pm_rr->type;
    freeReplyObject(pm_rr);
    return p;
}
#endif