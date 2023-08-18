// 服务器对客户端personalmenu里不同的选项进行不同的处理
#ifndef PERSONALPROCESS_HPP
#define PERSONALPROCESS_HPP

#include "../others/data.h"
#include "../others/define.h"
#include "../others/IO.h"
#include "../others/redis.h"
#include <vector>

#include <iostream>
using json = nlohmann::json;
using namespace std;

// 展示未通知消息
void showunreadnotice_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    printf("--- %s 用户显示离线未读消息 ---\n", friend_.id.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key = friend_.id + ":unreadnotice";
    int len = redis.scard(key);

    redisReply **arry = redis.smembers(key);
    vector<string> unreadnotice_Vector; // 放未读消息的容器

    // 把数据从数据库转移到容器里
    for (int i = 0; i < len; i++)
    {
        // 得到未读消息
        string unreadnotice = arry[i]->str;
        unreadnotice_Vector.push_back(unreadnotice);

        freeReplyObject(arry[i]);
        redis.sremvalue(key, unreadnotice);
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"vector", unreadnotice_Vector},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 退出登录
void logout_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    string id = parsed_data["id"];
    printf("--- %s 用户将要退出登录 ---\n", id.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string userjson_string;
    userjson_string = redis.gethash("userinfo", id);
    parsed_data = json::parse(userjson_string);

    // 改变用户的在线状态
    parsed_data["online"] = OFFLINE;
    userjson_string = parsed_data.dump();
    redis.hsetValue("userinfo", id, userjson_string);
    redis.sremvalue("onlinelist", id); // 把用户从在线列表中移除
    redis.hashdel("usersocket_id", redis.gethash("usersocket", id));
    redis.hashdel("usersocket", id); // 把用户的套接字移除

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", 0},
        {"flag", LOGOUT},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);

    cout << "退出登录成功" << endl;
}

// 加好友
void addfriend_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    friend_.oppoid = parsed_data["oppoid"];
    printf("--- %s 用户将向 %s 发送好友申请 ---\n", friend_.id.c_str(), friend_.oppoid.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    // 构造好友列表
    string key = friend_.id + ":friends";            // id+friends作为键，值就是id用户的好友们
    string key_ = friend_.oppoid + ":friends_apply"; // 对方的好友申请表
    string unkey = friend_.oppoid + ":unreadnotice"; // 未读通知

    // 加好友
    if (redis.hashexists("userinfo", friend_.oppoid) != 1) // 账号不存在
    {
        cout << "该id不存在，请重新输入" << endl;
        friend_.state = USERNAMEUNEXIST;
        friend_.type = NORMAL;
    }
    else if (redis.sismember(key, friend_.oppoid) == 1) // 好友列表里已有对方
    {
        cout << "你们已经是好友" << endl;
        friend_.state = HADFRIEND;
        friend_.type = NORMAL;
    }
    else if (redis.sismember("onlinelist", friend_.oppoid) == 1) // 在线列表里有对方
    {
        cout << "对方在线" << endl;
        friend_.msg = redis.gethash("id_name", friend_.id) + "向你发送了一条好友申请";
        friend_.state = SUCCESS;
        friend_.type = NOTICE;

        // 放到对方的好友申请表中
        redis.saddvalue(key_, friend_.id);
    }
    else // 对方不在线：加入数据库，等用户上线时提醒
    {
        cout << "对方不在线" << endl;
        friend_.msg = redis.gethash("id_name", friend_.id) + "向你发送了一条好友申请";
        friend_.state = SUCCESS;
        friend_.type = NORMAL; // 对方不在线，不能及时通知，因此设为普通事件，让用户知道已经发送了好友申请

        // 加入到对方的未读通知消息队列里
        redis.saddvalue(unkey, friend_.msg);

        // 放到对方的好友申请表中
        redis.saddvalue(key_, friend_.id);
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", friend_.type},
        {"state", friend_.state},
        {"msg", friend_.msg},
        {"flag", 0},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    if (friend_.type == NORMAL)
    {
        sendmsg.SendMsg_client(fd, json_string);
    }
    else if (friend_.type == NOTICE) // 如果是通知消息，那就把这条消息发给对方（所以下面要根据对方的id获得对方的socket）
    {
        sendmsg.SendMsg_client(stoi(redis.gethash("usersocket", friend_.oppoid)), json_string);

        // 改成正常的类型后给本用户的客户端发回去，不然客户端接不到事件的处理进度
        json_["type"] = NORMAL;
        json_string = json_.dump();
        sendmsg.SendMsg_client(fd, json_string);
    }
    cout << "here" << endl;
}

// 好友申请----->在函数里用到了recv，如果是非阻塞的话，recv会显示接受失败，然后就和那个客户端断开了连接
// 查看申请列表
void friendapplylist_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    printf("--- %s 用户查看好友申请列表 ---\n", friend_.id.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key = friend_.id + ":friends_apply";
    int len = redis.scard(key);

    redisReply **arry = redis.smembers(key);
    vector<string> friendapply_Vector; // 放好友请求的容器

    // 把数据从数据库转移到容器里
    for (int i = 0; i < len; i++)
    {
        // 得到发送请求的用户id
        string applyfriend_id = arry[i]->str;
        string name = redis.gethash("id_name", applyfriend_id); // 拿着id去找username

        friendapply_Vector.push_back(name);

        freeReplyObject(arry[i]);
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"vector", friendapply_Vector},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}
// 编辑好友申请
void friendapplyedit_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    string name = parsed_data["name"];
    string state = parsed_data["state"];
    printf("--- %s 用户编辑好友申请 ---\n", friend_.id.c_str());
    cout << name << endl;
    cout << state << endl;

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key = friend_.id + ":friends_apply";

    if (redis.sismember("username", name) != 1)
    {
        cout << "查无此人" << endl;

        friend_.state = USERNAMEUNEXIST;
        friend_.type = NORMAL;
    }
    else
    {
        // 得到发送请求的用户id
        string applyfriend_id = redis.gethash("name_id", name); // 由昵称找id

        if (redis.sismember(key, applyfriend_id) != 1)
        {
            cout << "不存在此好友申请！" << endl;
            friend_.state = FAIL;
            friend_.type = NORMAL;
        }
        else if (state == "1")
        {
            cout << "已同意" << endl;
            redis.sremvalue(key, applyfriend_id); // 从申请列表中移除
            if (redis.sismember(applyfriend_id + ":friends_apply", friend_.id) == 1)
            {
                redis.sremvalue(applyfriend_id + ":friends_apply", friend_.id);
            }
            string key1 = friend_.id + ":friends";
            string key2 = applyfriend_id + ":friends";
            redis.saddvalue(key1, applyfriend_id); // 对方成为自己好友
            redis.saddvalue(key2, friend_.id);     // 自己成为对方好友

            friend_.state = SUCCESS;
            friend_.type = NORMAL;
        }
        else if (state == "2")
        {
            cout << "已拒绝" << endl;
            redis.sremvalue(key, applyfriend_id); // 从申请列表中移除
            if (redis.sismember(applyfriend_id + ":friends_apply", friend_.id) == 1)
            {
                redis.sremvalue(applyfriend_id + ":friends_apply", friend_.id);
            }

            friend_.state = SUCCESS;
            friend_.type = NORMAL;
        }
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", friend_.type},
        {"flag", 0},
        {"state", friend_.state},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 好友信息
void friendinfo_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    printf("--- %s 用户查看在线好友列表 ---\n", friend_.id.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key = friend_.id + ":friends";
    int len = redis.scard(key);

    redisReply **arry = redis.smembers(key);
    vector<string> friendsname_Vector; // 放好友姓名的容器
    vector<string> friendsid_Vector;   // 放好友id的容器
    vector<int> friendsonline_Vector;  // 放好友是否在线的容器（1在线，0不在）

    // 把数据从数据库转移到容器里
    for (int i = 0; i < len; i++)
    {
        // 得到用户id
        string friend_id = arry[i]->str;
        if (redis.hashexists("userinfo", friend_id) != 1) // id不存在，说明该用户已经注销了
            continue;

        string name = redis.gethash("id_name", friend_id); // 拿着id去找username

        friendsname_Vector.push_back(name);
        friendsid_Vector.push_back(friend_id);
        friendsonline_Vector.push_back(redis.sismember("onlinelist", friend_id) == 1);

        freeReplyObject(arry[i]);
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"namevector", friendsname_Vector},
        {"idvector", friendsid_Vector},
        {"onlinevector", friendsonline_Vector},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 屏蔽好友
void addblack_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    friend_.oppoid = parsed_data["oppoid"];
    printf("--- %s 用户将要屏蔽好友 ---\n", friend_.id.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key = friend_.id + ":bfriends";
    string key_ = friend_.id + ":friends";

    if (redis.sismember(key_, friend_.oppoid) != 1) // 对方不是你的好友
    {
        cout << "对方不是你好友" << endl;
        friend_.state = FAIL;
    }
    else if (redis.sismember(key, friend_.oppoid) == 1) // 已拉黑对方
    {
        cout << "已拉黑对方" << endl;
        friend_.state = HADBLACK;
    }
    else
    {
        cout << "拉黑成功" << endl;
        redis.saddvalue(key, friend_.oppoid);
        friend_.state = SUCCESS;
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"state", friend_.state},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 删除好友
void delfriend_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    friend_.oppoid = parsed_data["oppoid"];
    printf("--- %s 用户将要删除好友 ---\n", friend_.id.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key = friend_.id + ":friends";
    string key_ = friend_.oppoid + ":friends";
    string bkey = friend_.id + ":bfriends";

    //*************还要删除聊天记录****************
    if (redis.sismember(key, friend_.oppoid) == 1 && redis.sremvalue(key, friend_.oppoid) == 3 && redis.sremvalue(key_, friend_.id) == 3 && redis.sremvalue(bkey, friend_.oppoid))
    {
        cout << "删除成功" << endl;
        friend_.state = SUCCESS;
    }
    else
    {
        cout << "删除失败" << endl;
        friend_.state = FAIL;
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"state", friend_.state},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 查看屏蔽好友列表
void blackfriendlist_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    printf("--- %s 用户查看屏蔽好友列表 ---\n", friend_.id.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key = friend_.id + ":bfriends";
    int len = redis.scard(key);

    redisReply **arry = redis.smembers(key);
    vector<string> bfriends_Vector; // 放黑名单的容器

    // 把数据从数据库转移到容器里
    for (int i = 0; i < len; i++)
    {
        // 得到发送请求的用户id
        string bfriend_id = arry[i]->str;
        if (redis.hashexists("userinfo", bfriend_id) != 1 || redis.sismember(friend_.id + ":friends", bfriend_id) != 1) // id不存在，说明该用户已经注销了
            continue;

        string name = redis.gethash("id_name", bfriend_id); // 拿着id去找username

        bfriends_Vector.push_back(name);

        freeReplyObject(arry[i]);
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"vector", bfriends_Vector},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}
// 编辑屏蔽好友
void blackfriendedit_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    string name = parsed_data["name"];
    printf("--- %s 用户编辑屏蔽好友 ---\n", friend_.id.c_str());
    cout << name << endl;

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key = friend_.id + ":bfriends";

    if (redis.sismember("username", name) != 1)
    {
        cout << "查无此人" << endl;

        friend_.state = USERNAMEUNEXIST;
        friend_.type = NORMAL;
    }
    else
    {
        // 得到发送请求的用户id
        string bfriend_id = redis.gethash("name_id", name); // 由昵称找id

        if (redis.sismember(key, bfriend_id) != 1)
        {
            cout << "不存在此拉黑好友！" << endl;
            friend_.state = FAIL;
            friend_.type = NORMAL;
        }
        else
        {
            cout << "已从拉黑名单中去除" << endl;
            redis.sremvalue(key, bfriend_id); // 从屏蔽列表中移除

            friend_.state = SUCCESS;
            friend_.type = NORMAL;
        }
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", friend_.type},
        {"flag", 0},
        {"state", friend_.state},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 好友聊天历史记录
void historychat_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    friend_.opponame = parsed_data["opponame"];
    printf("--- %s 用户查看与 %s 的聊天记录 ---\n", friend_.id.c_str(), friend_.opponame.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string key;
    string key_ = friend_.id + ":friends";
    vector<string> historychat_Vector; // 放聊天记录的容器

    if (redis.sismember("username", friend_.opponame) != 1)
    {
        cout << "账号不存在" << endl;
        friend_.state = USERNAMEUNEXIST;
    }
    else if (redis.sismember(key_, redis.gethash("name_id", friend_.opponame)) != 1) // 对方不是你的好友
    {
        cout << "对方不是你好友" << endl;
        friend_.state = FAIL;
    }
    else
    {
        friend_.oppoid = redis.gethash("name_id", friend_.opponame); // 用户注销完会从该哈希表中去除，所以用name找到的id也是独一无二的
        if (friend_.id < friend_.oppoid)
        {
            key = friend_.id + friend_.oppoid + ":historychat";
        }
        else
        {
            key = friend_.oppoid + friend_.id + ":historychat";
        }

        int len = redis.llen(key);
        redisReply **arry = redis.lrange(key);

        // 把数据从数据库转移到容器里
        for (int i = 0; i < len; i++)
        {
            // 得到历史消息json的字符串
            string msg = arry[i]->str;

            historychat_Vector.push_back(msg);

            freeReplyObject(arry[i]);
        }
        friend_.state = SUCCESS;
    }
    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"vector", historychat_Vector},
        {"state", friend_.state},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 与好友聊天
void chatfriend_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    friend_.opponame = parsed_data["opponame"];
    friend_.msg = parsed_data["msg"];
    printf("--- %s 用户向 %s 发送聊天消息 ---\n", friend_.id.c_str(), friend_.opponame.c_str());
    cout << friend_.msg << endl;

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");
    nlohmann::json json_;

    friend_.oppoid = redis.gethash("name_id", friend_.opponame);

    // 看是否拉黑对方和被对方拉黑
    string bkey = friend_.id + ":bfriends";
    string oppobkey = friend_.oppoid + ":bfriends";
    if (redis.sismember(bkey, friend_.oppoid) == 1 || redis.sismember(oppobkey, friend_.id) == 1) // 被拉黑
    {
        friend_.type = NORMAL;
        friend_.state = FAIL;
    }
    else
    {
        string key;
        string unkey = friend_.oppoid + ":unreadnotice"; // 未读通知

        if (friend_.id < friend_.oppoid)
        {
            key = friend_.id + friend_.oppoid + ":historychat";
        }
        else
        {
            key = friend_.oppoid + friend_.id + ":historychat";
        }

        // 将名字和聊天内容json成字符串msg，存入list中
        json_ = {
            {"name", redis.gethash("id_name", friend_.id)},
            {"msg", friend_.msg},
        };
        string msg = json_.dump();
        redis.lpush(key, msg);

        // 对方是否在线
        if (redis.sismember("onlinelist", friend_.oppoid) == 1) // 在线
        {
            cout << "对方在线" << endl;
            friend_.type = NOTICE;
        }
        else // 不在线：加入数据库，等用户上线时提醒
        {
            cout << "对方不在线" << endl;
            redis.saddvalue(unkey, redis.gethash("id_name", friend_.id) + "发来新消息"); // 加入到对方的未读通知消息队列里
            friend_.type = NORMAL;
        }
    }

    // 发送状态和信息类型
    json_ = {
        {"type", friend_.type},
        {"flag", PRIVATE},
        {"state", friend_.state},
        {"msg", friend_.msg},
        {"id", friend_.id},
        {"name", redis.gethash("id_name", friend_.id)},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    if (friend_.type == NOTICE)
    {
        sendmsg.SendMsg_client(stoi(redis.gethash("usersocket", friend_.oppoid)), json_string);

        json_["type"] = NORMAL;
        json_string = json_.dump();
        sendmsg.SendMsg_client(fd, json_string);
    }
    else
    {
        sendmsg.SendMsg_client(fd, json_string);
    }

    cout << "here" << endl;
}

// 个人信息
void personalinfo_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Friend friend_;
    friend_.id = parsed_data["id"];
    printf("--- %s 用户查看个人信息 ---\n", friend_.id.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    string userjson_string;
    userjson_string = redis.gethash("userinfo", friend_.id);
    parsed_data = json::parse(userjson_string);

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"username", parsed_data["username"]},
        {"password", parsed_data["password"]},
        {"secrecy", parsed_data["secrecy"]},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

#endif