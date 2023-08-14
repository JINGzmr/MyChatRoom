// 服务器对客户端groupmenu里不同的选项进行不同的处理
#ifndef GROUPPROCESS_HPP
#define GROUPPROCESS_HPP

#include "data.h"
#include "define.h"
#include "IO.h"
#include "redis.hpp"
#include <vector>

#include <iostream>
using json = nlohmann::json;
using namespace std;

// 创建群组
void creatgroup_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Group group;
    group.groupname = parsed_data["groupname"];
    group.ownerid = parsed_data["ownerid"];
    printf("--- %s 用户将要创建名称为 %s 的群聊 ---\n", group.ownerid.c_str(), group.groupname.c_str());

    Redis redis;
    redis.connect();

    if (redis.sismember("groupname", group.groupname) == 1) // 名称已被使用
    {
        cout << "该群组名称已存在，请更改名称后重新创建" << endl;
        group.state = USERNAMEEXIST;
    }
    else
    {
        // 生成群id
        group.groupid = produce_id();

        // 向数据库里加数据
        int m = redis.saddvalue("groupname", group.groupname);                   // 所有的群聊名称
        int n = redis.hsetValue("groupname_id", group.groupname, group.groupid); // 群名找群id
        int o = redis.hsetValue("groupid_name", group.groupid, group.groupname); // 群id找群名
        int p = redis.saddvalue(group.ownerid + ":group", group.groupid);        // id对应用户所加的群聊
        int q = redis.saddvalue(group.groupid + ":num", group.ownerid);          // 群成员名单
        int r = redis.saddvalue(group.groupid + ":admin", group.ownerid);        // 群管理员
        int s = redis.saddvalue(group.ownerid + ":mycreatgroup", group.groupid); // id对应用户创建的群聊
        int t = redis.saddvalue(group.ownerid + ":myadmingroup", group.groupid); // id对应用户管理的群聊（包括当群主的群聊）

        if (n == REDIS_REPLY_ERROR || m == REDIS_REPLY_ERROR || o == REDIS_REPLY_ERROR || p == REDIS_REPLY_ERROR || q == REDIS_REPLY_ERROR || r == REDIS_REPLY_ERROR || s == REDIS_REPLY_ERROR || t == REDIS_REPLY_ERROR)
        {
            cout << "创建失败" << endl;
            group.state = FAIL;
        }
        else
        {
            cout << "创建成功" << endl;
            group.state = SUCCESS;
        }
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"state", group.state},
        {"flag", 0},
        {"groupid", group.groupid},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 加入群组(通知管理员)
void addgroup_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Group group;
    group.groupid = parsed_data["groupid"];
    group.userid = parsed_data["userid"];
    printf("--- %s 用户将向 %s 发送加群申请 ---\n", group.userid.c_str(), group.groupid.c_str());

    Redis redis;
    redis.connect();

    string key = group.groupid + ":admin";
    int len = redis.scard(key);
    redisReply **arry = redis.smembers(key);
    vector<string> admins_Vector; // 放群主及管理员的的容器
    vector<string> online_Vector; // 在线的管理员名单
    int online_len = 0;           // 名单长度

    // 加群
    if (redis.hashexists("groupid_name", group.groupid) != 1) // 群id不存在
    {
        cout << "该群id不存在" << endl;
        group.state = USERNAMEUNEXIST;
        group.type = NORMAL;
    }
    else if (redis.sismember(group.userid + ":group", group.groupid) == 1) // 已加入该群
    {
        cout << "你已加入该群！" << endl;
        group.state = HADINGROUP;
        group.type = NORMAL;
    }
    else // 可以申请
    {
        // 找出群管理员
        for (int i = 0; i < len; i++)
        {
            string admin_id = arry[i]->str;
            admins_Vector.push_back(admin_id);

            if (redis.sismember("onlinelist", admin_id) == 1) // 在线列表里有群管理员，那就给每个在线的发送通知
            {
                cout << admin_id << "在线" << endl;
                online_Vector.push_back(admin_id);
                online_len++;
                group.msg = redis.gethash("id_name", group.userid) + "向“" + redis.gethash("groupid_name", group.groupid) + "”发送了一条加群申请";
                group.state = SUCCESS;
                group.type = NOTICE;

                // 放到该群的申请表中
                redis.saddvalue(group.groupid + ":groupapply", group.userid);
            }
            else // 对方不在线：加入数据库，等管理员上线时提醒，如果被其他管理员同意了，进入申请进群列表里无法看到(同意时要把这个从申请表中移除)
            {
                cout << admin_id << "不在线" << endl;
                group.msg = redis.gethash("id_name", group.userid) + "向“" + redis.gethash("groupid_name", group.groupid) + "”发送了一条加群申请";
                group.state = SUCCESS;
                group.type = NORMAL;

                // 加入到对方的未读通知消息队列里
                redis.saddvalue(admin_id + ":unreadnotice", group.msg);

                // 放到该群的申请表中
                redis.saddvalue(group.groupid + ":groupapply", group.userid);
            }
            freeReplyObject(arry[i]);
        }
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", group.type},
        {"state", group.state},
        {"msg", group.msg},
        {"flag", 0},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;

    if (group.type == NORMAL)
    {
        sendmsg.SendMsg_client(fd, json_string);
    }
    else if (group.type == NOTICE) // 如果是通知消息，那就把这条消息发给管理员（下面根据管理员的id获得socket）
    {
        for (int i = 0; i < online_len; i++)
        {
            string onlineadmin_id = online_Vector[i];
            sendmsg.SendMsg_client(stoi(redis.gethash("usersocket", onlineadmin_id)), json_string);
        }
        // 改成正常的类型后给本用户的客户端发回去，不然客户端接不到事件的处理进度
        json_["type"] = NORMAL;
        json_string = json_.dump();
        sendmsg.SendMsg_client(fd, json_string);
    }
    cout << "here" << endl;
}

// 查看已加入的群组
void checkgroup_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Group group;
    group.userid = parsed_data["userid"];
    printf("--- %s 用户查看已加入的群组 ---\n", group.userid.c_str());
cout<< "1"<< endl;
    Redis redis;
    redis.connect();
cout<< "2"<< endl;

    string key = group.userid + ":group";
    int len = redis.scard(key);
cout<< "3"<< endl;

    redisReply **arry = redis.smembers(key);
    vector<string> groupname_Vector;  // 放群名的容器
    vector<string> groupid_Vector;    // 放群id的容器
    vector<int> groupposition_Vector; // 放该用户在目标群职位的容器
    // 将容器置空
    groupname_Vector.clear();
    groupid_Vector.clear();
    groupposition_Vector.clear();
cout<< "4"<< endl;

    // 把数据从数据库转移到容器里
    for (int i = 0; i < len; i++)
    {
        // 得到群id
        string groupid = arry[i]->str;
        if (redis.hashexists("groupid_name", groupid) != 1) // 群id不存在，说明该群已经解散了，但在该用户加入的群的数据库里仍存着（不好删）
            continue;

        string name = redis.gethash("groupid_name", groupid); // 拿着id去找name

        groupname_Vector.push_back(name);
        groupid_Vector.push_back(groupid);
        if (redis.sismember(group.userid + ":mycreatgroup", groupid) == 1)
        {
            groupposition_Vector.push_back(2);
        }
        else if (redis.sismember(group.userid + ":myadmingroup", groupid) == 1)
        {
            groupposition_Vector.push_back(1);
        }
        else
        {
            groupposition_Vector.push_back(0);
        }

        freeReplyObject(arry[i]);
    }
cout<< "5"<< endl;

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"groupnamevector", groupname_Vector},
        {"groupidvector", groupid_Vector},
        {"grouppositionvector", groupposition_Vector},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
cout<< "6"<< endl;

}

// 退出已加入的群组（就不通知管理员了）
void outgroup_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Group group;
    group.groupid = parsed_data["groupid"];
    group.userid = parsed_data["userid"];
    printf("--- %s 用户将要退出 %s 群 ---\n", group.userid.c_str(), group.groupid.c_str());

    Redis redis;
    redis.connect();

    if (redis.hashexists("groupid_name", group.groupid) == 1 && redis.sismember(group.userid + ":group", group.groupid) == 1) // 存在该群且已加入
    {
        if (redis.sismember(group.userid + ":mycreatgroup", group.groupid) == 1) // 自己是群组，则解散该群
        {
            group.groupname = redis.gethash("groupid_name", group.groupid);
            int m = redis.sremvalue("groupname", group.groupname);  // 所有的群聊名称
            int n = redis.hashdel("groupname_id", group.groupname); // 群名找群id
            int o = redis.hashdel("groupid_name", group.groupid);   // 群id找群名
            // int s = redis.sremvalue(group.userid + "mycreatgroup", group.groupid); // id对应用户创建的群聊
        }
        else
        {
            int p = redis.sremvalue(group.userid + ":group", group.groupid);         // id对应用户所加的群聊
            int q = redis.sremvalue(group.groupid + ":num", group.userid);           // 群成员名单
            if (redis.sismember(group.userid + ":myadmingroup", group.groupid) == 1) // 自己是管理员
            {
                int r = redis.sremvalue(group.groupid + ":admin", group.userid);        // 群管理员
                int t = redis.sremvalue(group.userid + ":myadmingroup", group.groupid); // id对应用户管理的群聊（包括当群主的群聊）
            }
        }
        cout << "退出成功" << endl;
        group.state = SUCCESS;
    }
    //*************还要删除群聊记录****************
    else
    {
        cout << "退出失败" << endl;
        group.state = FAIL;
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"state", group.state},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

//  查看群组成员列表
void checkgroupnum_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Group group;
    group.userid = parsed_data["userid"];
    group.groupid = parsed_data["groupid"];
    printf("--- %s 用户查看 %s 群的成员 ---\n", group.userid.c_str(), group.groupid.c_str());

    Redis redis;
    redis.connect();

    vector<string> groupnumname_Vector;  // 放群成员名字的容器
    vector<string> groupnumid_Vector;    // 放群成员id的容器
    vector<int> groupnumposition_Vector; // 放群成员职位的容器

    if (redis.hashexists("groupid_name", group.groupid) != 1) // 群id不存在
    {
        cout << "该群id不存在" << endl;
        group.state = USERNAMEUNEXIST;
    }
    else if (redis.sismember(group.userid + ":group", group.groupid) != 1) // 已加入该群
    {
        cout << "你未加入该群！" << endl;
        group.state = FAIL;
    }
    else
    {
        string key = group.groupid + ":num";
        int len = redis.scard(key);

        redisReply **arry = redis.smembers(key);

        // 把数据从数据库转移到容器里
        for (int i = 0; i < len; i++)
        {
            // 得到用户id
            string groupnumid = arry[i]->str;
            if (redis.hashexists("userinfo", groupnumid) != 1) // 用户id不存在，说明该用户已注销，但在该用户加入的群的数据库里仍存着（不好删）
                continue;

            string name = redis.gethash("id_name", groupnumid); // 拿着id去找name

            groupnumname_Vector.push_back(name);
            groupnumid_Vector.push_back(groupnumid);
            if (redis.sismember(groupnumid + ":mycreatgroup", group.groupid) == 1)
            {
                groupnumposition_Vector.push_back(2);
            }
            else if (redis.sismember(groupnumid + ":myadmingroup", group.groupid) == 1)
            {
                groupnumposition_Vector.push_back(1);
            }
            else
            {
                groupnumposition_Vector.push_back(0);
            }

            freeReplyObject(arry[i]);
        }
        group.state = SUCCESS;
    }

    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"state", group.state},
        {"groupnumnamevector", groupnumname_Vector},
        {"groupnumidvector", groupnumid_Vector},
        {"groupnumpositionvector", groupnumposition_Vector},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 查看群组聊天记录
void historygroupchat_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Group group;
    group.groupid = parsed_data["groupid"];
    group.userid = parsed_data["userid"];
    printf("--- %s 用户将要查看 %s 群的历史消息 ---\n", group.userid.c_str(), group.groupid.c_str());

    Redis redis;
    redis.connect();

    string key = group.groupid + ":historygroupchat";
    vector<string> historygroupchat_Vector; // 放聊天记录的容器

    if (redis.hashexists("groupid_name", group.groupid) == 1 && redis.sismember(group.userid + ":group", group.groupid) == 1) // 存在该群且已加入
    {
        int len = redis.llen(key);
        redisReply **arry = redis.lrange(key);

        // 把数据从数据库转移到容器里
        for (int i = 0; i < len; i++)
        {
            // 得到历史消息json的字符串
            string msg = arry[i]->str;

            historygroupchat_Vector.push_back(msg);

            freeReplyObject(arry[i]);
        }
        group.state = SUCCESS;
    }
    else
    {
        cout << "查看失败" << endl;
        group.state = FAIL;
    }
    // 发送状态和信息类型
    nlohmann::json json_ = {
        {"type", NORMAL},
        {"flag", 0},
        {"vector", historygroupchat_Vector},
        {"state", group.state},
    };
    string json_string = json_.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(fd, json_string);
}

// 选择群组聊天
void groupchat_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct Group group;
    group.groupid = parsed_data["groupid"];
    group.userid = parsed_data["userid"];
    group.msg = parsed_data["msg"];
    printf("--- %s 用户向 %s 群组发送聊天消息 ---\n", group.userid.c_str(), group.groupid.c_str());
    cout << group.msg << endl;

    Redis redis;
    redis.connect();
    nlohmann::json json_;

    // 看是否还在群组中（会不会聊天到一半被踢出群聊或群聊被解散）
    if (redis.hashexists("groupid_name", group.groupid) == 1 && redis.sismember(group.userid + ":group", group.groupid) == 1) // 该群存在且已加入
    {
        group.groupname = redis.gethash("groupid_name", group.groupid);
        string key = group.groupid + ":num";
        string historykey = group.groupid + ":historygroupchat";
        int len = redis.scard(key);
        redisReply **arry = redis.smembers(key);

        json_ = {
            {"name", redis.gethash("id_name", group.userid)},
            {"msg", group.msg},
        };
        string msg = json_.dump();
        redis.lpush(historykey, msg);

        // 把数据从数据库转移到容器里
        for (int i = 0; i < len; i++)
        {
            // 得到群聊成员id
            string groupnumid = arry[i]->str;
            if (redis.hashexists("userinfo", groupnumid) != 1) // 用户id不存在，说明该用户已注销，但在该用户加入的群的数据库里仍存着（不好删）
                continue;

            string unkey = groupnumid + ":unreadnotice"; // 该群所有人的未读通知

            // 群聊每个人是否在线
            if (redis.sismember("onlinelist", groupnumid) == 1) // 在线
            {
                cout << redis.gethash("id_name", groupnumid) << "在线" << endl;
                group.type = NOTICE;
            }
            else // 不在线：加入数据库，等用户上线时提醒
            {
                cout << redis.gethash("id_name", groupnumid) << "不在线" << endl;
                redis.saddvalue(unkey, group.groupname + "有新消息"); // 加入到对方的未读通知消息队列里
                group.type = NORMAL;
            }

            if (groupnumid == group.userid) // 别再给自己通知一遍了
            {
                group.type = NORMAL;
            }
                
            // 发送状态和信息类型
            json_ = {
                {"type", group.type},
                {"state", group.state},
                {"msg", group.msg},
                {"name", redis.gethash("id_name", group.userid)},
                {"groupname", group.groupname},
                {"groupid", group.groupid},
                {"flag", GROUP},
            };
            string json_string = json_.dump();
            SendMsg sendmsg;
            if (group.type == NOTICE)
            {
                sendmsg.SendMsg_client(stoi(redis.gethash("usersocket", groupnumid)), json_string);
            }
            else
            {
                sendmsg.SendMsg_client(fd, json_string);
            }
        }
    }
    else
    {
        group.type = NORMAL;
        group.state = FAIL;

        // 发送状态和信息类型
        json_ = {
            {"type", group.type},
            {"state", group.state},
            {"flag", 0},
        };
        string json_string = json_.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(fd, json_string);
    }
    cout << "here" << endl;
}

#endif