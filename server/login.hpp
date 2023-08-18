// 服务端处理客户端在登录界面输入的注册、登入、注销
#include "../others/data.h"
#include "../others/define.h"
#include "../others/redis.h"
#include "../others/IO.h"

#include <iostream>
using json = nlohmann::json;
using namespace std;

// 登录
void login_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct User user;
    user.id = parsed_data["id"];
    user.password = parsed_data["password"];
    printf("--- %s 用户将要登录 ---\n", user.id.c_str());

    // 下面两种都行，但第一种的话要加上.c_str()
    printf("id：%s\n密码：%s\n", user.id.c_str(), user.password.c_str());
    // cout << user.username << '\n' << user.password << endl;

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    if (redis.hashexists("userinfo", user.id) != 1) // 账号不存在
    {
        cout << "该id不存在，请注册 或 重新输入" << endl;
        SendMsg sendmsg;
        sendmsg.SendMsg_int(fd, USERNAMEUNEXIST);
    }
    else // 账号存在
    {
        string userjson_string;
        userjson_string = redis.gethash("userinfo", user.id);
        parsed_data = json::parse(userjson_string);
        if (user.password != parsed_data["password"])
        {
            cout << "密码错误" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, FAIL);
        }
        else if (redis.sismember("onlinelist", user.id) == 1)
        {
            cout << "用户已登录" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, ONLINE);
        }
        else
        {
            cout << "登录成功" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, SUCCESS);

            // 改变用户的在线状态（将原来的string只改变在线状态后，完整地发送给数据库）
            // int state_ = parsed_data["online"];
            // state_ = ONLINE;
            parsed_data["online"] = ONLINE;
            userjson_string = parsed_data.dump();
            redis.hsetValue("userinfo", user.id, userjson_string);
            redis.saddvalue("onlinelist", user.id);                // 在线列表
            redis.hsetValue("usersocket", user.id, to_string(fd)); // 存套接字（用to_string将int类型转换成string类型）-->客户端与服务器连接时才会产生套接字，所以只记录在线用户的套接字（则也只是为了能实时通知）
            redis.hsetValue("usersocket_id", to_string(fd), user.id);
            //------------登录成功后，是客户端进入下一步，服务端只需要根据客户端发来的请求 调用相应的函数 来处理即可-------------
            // 现在这个登录的任务服务器处理完毕了，也就可以返回上一级了
            // 该线程也就被回收了^_^
        }
    }

    return;
}

// 注册
void register_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct User user;
    user.username = parsed_data["username"];
    user.password = parsed_data["password"];
    user.secrecy = parsed_data["password"];
    printf("--- %s 用户将要注册 ---\n", user.username.c_str());
    printf("用户名：%s\n密码：%s\n密保：%s", user.username.c_str(), user.password.c_str(), user.secrecy.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    if (redis.sismember("username", user.username) == 1) // 用户名已被使用
    {
        cout << "该用户名已存在，请登录 或 更改用户名后重新注册" << endl;
        SendMsg sendmsg;
        sendmsg.SendMsg_int(fd, USERNAMEEXIST);
    }
    else // 新的用户名，可以被使用
    {
        // 随机生成id
        string id = produce_id();

        int n = redis.hsetValue("userinfo", id, buf);
        int m = redis.saddvalue("username", user.username);
        int o = redis.hsetValue("id_name", id, user.username);
        int p = redis.hsetValue("name_id", user.username, id);

        if (n == REDIS_REPLY_ERROR || m == REDIS_REPLY_ERROR || o == REDIS_REPLY_ERROR || p == REDIS_REPLY_ERROR)
        {
            cout << "注册失败" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, FAIL);
        }
        else
        {
            cout << "注册成功" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, SUCCESS);
            sendmsg.SendMsg_client(fd, id);
        }
    }
    return;
}

// 注销
void signout_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct User user;
    user.id = parsed_data["id"];
    user.password = parsed_data["password"];
    printf("--- %s 用户将要注销 ---\n", user.id.c_str());
    printf("id：%s\n密码：%s\n", user.id.c_str(), user.password.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    if (redis.hashexists("userinfo", user.id) != 1) // 账号不存在
    {
        cout << "该用户名不存在，请注册 或 重新输入" << endl;
        SendMsg sendmsg;
        sendmsg.SendMsg_int(fd, USERNAMEUNEXIST);
    }
    else // 账号存在
    {
        string userjson_string;
        userjson_string = redis.gethash("userinfo", user.id);
        parsed_data = json::parse(userjson_string);
        // ************还要把该用户从所有人的好友名单、拉黑名单、群聊名单里删去**********
        // *********或者，当用户要看这些名单时，可以做一个判断，如果这个id存在，则打印出来，不存在，则不打印，但数据库来还存在就是了************
        if (user.password == parsed_data["password"] && redis.hashdel("userinfo", user.id) == 3 && redis.sremvalue("username", parsed_data["username"]) == 3 && redis.hashdel("id_name", user.id) == 3 && redis.hashdel("name_id", parsed_data["username"]) == 3) // 密码正确且id从哈希表中成功移除、姓名从昵称表里移除
        {
            cout << "注销成功" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, SUCCESS);
        }
        else
        {
            cout << "注销失败" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, FAIL);
        }
    }
    return;
}

// 找回密码
void findpassword_server(int fd, string buf)
{
    json parsed_data = json::parse(buf);
    struct User user;
    user.id = parsed_data["id"];
    user.secrecy = parsed_data["secrecy"];
    printf("--- %s 用户将要找回密码 ---\n", user.id.c_str());
    printf("id：%s\n密码：%s\n", user.id.c_str(), user.secrecy.c_str());

    Redis redis;
    redis.connect("127.0.0.1", 6379, "");

    if (redis.hashexists("userinfo", user.id) != 1) // 账号不存在
    {
        cout << "该用户名不存在，请注册 或 重新输入" << endl;
        SendMsg sendmsg;
        sendmsg.SendMsg_int(fd, USERNAMEUNEXIST);
    }
    else // 账号存在
    {
        string userjson_string;
        userjson_string = redis.gethash("userinfo", user.id);
        parsed_data = json::parse(userjson_string);
        if (user.secrecy == parsed_data["secrecy"]) // 密保正确
        {
            cout << "成功" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, SUCCESS);
            sendmsg.SendMsg_client(fd, user.secrecy);
        }
        else
        {
            cout << "找回失败" << endl;
            SendMsg sendmsg;
            sendmsg.SendMsg_int(fd, FAIL);
        }
    }
    return;
}
