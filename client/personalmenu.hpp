// 客户端的聊天室界面，以及处理好友相关事件
#ifndef PERSONALMENU_HPP
#define PERSONALMENU_HPP

#include "../others/data.h"
#include "../others/define.h"
#include "../others/head.h"
#include "../others/IO.h"
#include "menu.hpp"
#include "../others/threadwork.hpp"
#include "groupmenu.hpp"
#include "filemenu.hpp"

#include <iostream>
using json = nlohmann::json;
using namespace std;

void showunreadnotice_client(int client_socket, string id, Queue<string> &RecvQue);
void logout_client(int client_socket, string username);
void addfriend_client(int client_socket, string username, Queue<string> &RecvQue);
void friendapplylist_client(int client_socket, string id, Queue<string> &RecvQue);
void friendapplyedit_client(int client_socket, string id, Queue<string> &RecvQue);
string friendinfo_client(int client_socket, string id, Queue<string> &RecvQue, int fl);
void addblack_client(int client_socket, string id, Queue<string> &RecvQue);
void delfriend_client(int client_socket, string id, Queue<string> &RecvQue);
string blackfriendlist_client(int client_socket, string id, Queue<string> &RecvQue, int fl);
void blackfriendedit_client(int client_socket, string id, Queue<string> &RecvQue);
string historychat_client(int client_socket, string id, Queue<string> &RecvQue, int fl);
void chatfriend_client(int client_socket, string id, Queue<string> &RecvQue);
void personalinfo_client(int client_socket, string id, Queue<string> &RecvQue);
void group_client(int client_socket, string id, Queue<string> &RecvQue);
void file_client(int client_socket, string id, Queue<string> &RecvQue);

void personalmenuUI(void)
{
    pid_t pid = getpid();
    cout << "————————————————————————————————————————————————————" << endl;
    cout << "---------------------  聊天室  --------------------" << endl;
    cout << "------------------ PID= " << pid << " ----------------------" <<endl;
    cout << "——————————————————————————————————————————————————" << endl;
    cout << "                      1.添加好友                   " << endl;
    cout << "                      2.编辑好友申请                " << endl;
    cout << "                      3.选择好友私聊                " << endl;
    cout << "                      4.查看历史聊天记录            " << endl;
    cout << "                      5.好友信息                   " << endl;
    cout << "                      6.屏蔽好友                   " << endl;
    cout << "                      7.删除好友                   " << endl;
    cout << "                      8.查看屏蔽好友列表             " << endl;
    cout << "                      9.编辑屏蔽好友                " << endl;
    cout << "                      10.个人信息                   " << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "                      11.群聊                      " << endl;
    cout << "---------------------------------------------------" << endl;
    cout << "                      12.文件传输                   " << endl;
    cout << "---------------------------------------------------" << endl;
    cout << "                      13.退出登录                   " << endl;
    cout << "---------------------------------------------------" << endl;
    cout << "                      0.刷新页面                  " << endl;
    cout << "———————————————————————————————————————————————————" << endl;
}

void messagemenu(int client_socket, string id, Queue<string> &RecvQue)
{
    system("clear");
    personalmenuUI();
    showunreadnotice_client(client_socket, id, RecvQue); // 展示离线消息

    int num = 1;
    do
    {
        // 清空缓冲区
        std::cin.clear();
        std::cin.sync();

        string str = getInputWithoutCtrlD();
        num = checkcin(str);

        switch (num)
        {
        case 1:
            system("clear");
            addfriend_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 2:
            system("clear");
            friendapplyedit_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 3:
            system("clear");
            chatfriend_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 4:
            system("clear");
            historychat_client(client_socket, id, RecvQue, 1);
            system("clear");
            personalmenuUI();
            break;
        case 5:
            system("clear");
            friendinfo_client(client_socket, id, RecvQue, 1);
            system("clear");
            personalmenuUI();
            break;
        case 6:
            system("clear");
            addblack_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 7:
            system("clear");
            delfriend_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 8:
            system("clear");
            blackfriendlist_client(client_socket, id, RecvQue, 1);
            system("clear");
            personalmenuUI();
            break;
        case 9:
            system("clear");
            blackfriendedit_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 10:
            system("clear");
            personalinfo_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 11:
            system("clear");
            group_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 12:
            system("clear");
            file_client(client_socket, id, RecvQue);
            system("clear");
            personalmenuUI();
            break;
        case 13:
            logout_client(client_socket, id);
            break;
        case 0:
            system("clear");
            personalmenuUI();
            break;
        default:
            cout << "无效的数字，请重新输入！" << endl;
        }
    } while (num != 13); // 13表示退出登录，即退出循环，返回上一级

    return;
}

// 展示未通知消息
void showunreadnotice_client(int client_socket, string id, Queue<string> &RecvQue)
{
    nlohmann::json sendJson_client = {
        {"flag", SHOUNOTICE},
        {"id", id},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    vector<string> unreadnotice_Vector = parsed_data["vector"];

    if (unreadnotice_Vector.empty())
    {
        cout << "—————————————无离线消息———————————————" << endl;
        return;
    }
    // 循环打印输出
    cout << "——————————————————————————————————————" << endl;
    for (const std::string &str : unreadnotice_Vector)
    {
        std::cout << str << std::endl;
    }
    cout << "—————————————以上为离线消息———————————————————" << endl;
}

// 退出登录
void logout_client(int client_socket, string id)
{
    nlohmann::json sendJson_client = {
        {"flag", LOGOUT},
        {"id", id},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);
    cout << "退出成功！" << endl;
    return;
}

// 加好友
void addfriend_client(int client_socket, string id, Queue<string> &RecvQue)
{
    Friend friend_;
    do
    {
        cout << "请输入你要添加的朋友ID：";
        friend_.oppoid = getInputWithoutCtrlD();
        friend_.id = id;
        friend_.flag = ADDFRIEND;

        if (friend_.id == friend_.oppoid)
        {
            cout << "不可添加自己！请重新输入！" << endl;
        }
    } while (friend_.id == friend_.oppoid);

    // 发送数据
    nlohmann::json sendJson_client = {
        {"id", friend_.id},
        {"oppoid", friend_.oppoid},
        {"flag", friend_.flag},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    int state_ = parsed_data["state"];

    // 判断是否发送成功
    if (state_ == HADFRIEND)
    {
        cout << "你们已经是好友！" << endl;
    }
    else if (state_ == USERNAMEUNEXIST)
    {
        cout << "该id不存在，请重新输入" << endl;
    }
    else if (state_ == SUCCESS)
    {
        cout << "已发送好友申请！" << endl;
    }

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
    return;
}

// 编辑好友申请
void friendapplyedit_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 查看好友申请
    // 发送数据
    nlohmann::json sendJson_client = {
        {"id", id},
        {"flag", FRIENDAPPLYLIST},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    vector<string> friendapply_Vector = parsed_data["vector"];

    if (friendapply_Vector.empty())
    {
        cout << "暂无好友申请！" << endl;
    }
    else
    {
        // 循环打印输出
        cout << "————————————以下为好友申请列表————————————" << endl;
        for (const std::string &str : friendapply_Vector)
        {
            std::cout << str << std::endl;
        }
        cout << "——————————————————————————————————————————" << endl;

        // 编辑好友申请
        string name;
        string state;
        cout << "输入要编辑的好友昵称" << endl;
        name = getInputWithoutCtrlD();
        cout << "同意---1 / 拒绝---0" << endl;
        state = getInputWithoutCtrlD();

        // 发送数据
        sendJson_client = {
            {"id", id},
            {"flag", FRIENDAPPLYEDIT},
            {"name", name},
            {"state", state},
        };
        sendJson_client_string = sendJson_client.dump();
        sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // 从消息队列里取消息
        buf = RecvQue.remove();
        parsed_data = json::parse(buf);
        int state_ = parsed_data["state"];

        // 判断是否操作成功
        if (state_ == SUCCESS)
        {
            cout << "操作成功！" << endl;
        }
        else if (state_ == FAIL)
        {
            cout << "不存在此好友申请！" << endl;
        }
        else if (state_ == USERNAMEUNEXIST)
        {
            cout << "查无此人! " << endl;
        }
        else
        {
            cout << "操作失败，请重新尝试！" << endl;
        }
    }

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
    return;
}

// 好友信息
string friendinfo_client(int client_socket, string id, Queue<string> &RecvQue, int fl)
{
    string str = "";
    // 发送数据
    nlohmann::json sendJson_client = {
        {"id", id},
        {"flag", FRIENDINFO},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    vector<string> friendsname_Vector = parsed_data["namevector"];
    vector<string> friendsid_Vector = parsed_data["idvector"];
    vector<int> friendsonline_Vector = parsed_data["onlinevector"];

    if (friendsname_Vector.empty())
    {
        cout << "暂无好友！" << endl;
        str = "fail";
    }
    else
    {
        // 循环打印输出
        cout << "————————————以下为好友列表————————————" << endl;
        for (int i = 0; i < friendsname_Vector.size(); i++)
        {
            cout << friendsname_Vector[i] << "  " << friendsid_Vector[i];
            if (friendsonline_Vector[i] == 1)
            {
                cout << "（在线）" << endl;
            }
            else
            {
                cout << '\n';
            }
        }
        cout << "——————————————————————————————————————————" << endl;
    }

    if (fl == 1)
    {
        cout << "按'q'返回上一级" << endl;
        string a;
        while ((a = getInputWithoutCtrlD()) != "q")
        {
        }
        return "";
    }

    return str;
}

// 屏蔽好友
void addblack_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 先打印出好友信息
    string re = friendinfo_client(client_socket, id, RecvQue, 0);

    if (re != "fail")
    {
        Friend friend_;
        do
        {
            cout << "请输入你要屏蔽的朋友ID：";
            friend_.oppoid = getInputWithoutCtrlD();
            friend_.id = id;
            friend_.flag = ADDBLACK;

            if (friend_.id == friend_.oppoid)
            {
                cout << "不可屏蔽自己！请重新输入！" << endl;
            }
        } while (friend_.id == friend_.oppoid);

        // 发送数据
        nlohmann::json sendJson_client = {
            {"id", friend_.id},
            {"oppoid", friend_.oppoid},
            {"flag", friend_.flag},
        };
        string sendJson_client_string = sendJson_client.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // 从消息队列里取消息
        string buf = RecvQue.remove();
        json parsed_data = json::parse(buf);
        int state_ = parsed_data["state"];

        // 判断是否拉黑成功
        if (state_ == SUCCESS)
        {
            cout << "拉黑成功！" << endl;
        }
        else if (state_ == FAIL)
        {
            cout << "对方不是你的好友！" << endl;
        }
        else if (state_ == HADBLACK)
        {
            cout << "对方已被拉黑！" << endl;
        }
    }

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
    return;
}

// 删除好友
void delfriend_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 先打印出好友信息
    string re = friendinfo_client(client_socket, id, RecvQue, 0);

    if (re != "fail")
    {
        Friend friend_;
        do
        {
            cout << "请输入你要删除的朋友ID：";
            friend_.oppoid = getInputWithoutCtrlD();
            friend_.id = id;
            friend_.flag = DELFRIEND;

            if (friend_.id == friend_.oppoid)
            {
                cout << "不可删除自己！请重新输入！" << endl;
            }
        } while (friend_.id == friend_.oppoid);

        // 发送数据
        nlohmann::json sendJson_client = {
            {"id", friend_.id},
            {"oppoid", friend_.oppoid},
            {"flag", friend_.flag},
        };
        string sendJson_client_string = sendJson_client.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // 从消息队列里取消息
        string buf = RecvQue.remove();
        json parsed_data = json::parse(buf);
        int state_ = parsed_data["state"];

        // 判断是否登入成功
        if (state_ == SUCCESS)
        {
            cout << "删除成功！" << endl;
        }
        else if (state_ == FAIL)
        {
            cout << "删除失败！" << endl;
        }
    }

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
    return;
}

// 查看屏蔽好友
string blackfriendlist_client(int client_socket, string id, Queue<string> &RecvQue, int fl)
{
    string str = "";
    // 发送数据
    nlohmann::json sendJson_client = {
        {"id", id},
        {"flag", BLACKFRIENDLIST},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    vector<string> bfriends_Vector = parsed_data["vector"];

    if (bfriends_Vector.empty())
    {
        cout << "暂无拉黑好友！" << endl;
        str = "fail";
    }
    // 循环打印输出
    cout << "————————————以下为拉黑好友列表————————————" << endl;
    for (const std::string &str : bfriends_Vector)
    {
        std::cout << str << std::endl;
    }
    cout << "——————————————————————————————————————————" << endl;

    if (fl == 1)
    {
        cout << "按'q'返回上一级" << endl;
        string a;
        while ((a = getInputWithoutCtrlD()) != "q")
        {
        }
        return "";
    }

    return str;
}
// 编辑屏蔽好友
void blackfriendedit_client(int client_socket, string id, Queue<string> &RecvQue)
{
    string re = blackfriendlist_client(client_socket, id, RecvQue, 0);
    if (re != "fail")
    {
        string name;
        int state;
        cout << "输入要移除黑名单的好友昵称" << endl;
        name = getInputWithoutCtrlD();

        // 发送数据
        nlohmann::json sendJson_client = {
            {"id", id},
            {"flag", BLACKFRIENDEDIT},
            {"name", name},
        };
        string sendJson_client_string = sendJson_client.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // 从消息队列里取消息
        string buf = RecvQue.remove();
        json parsed_data = json::parse(buf);
        int state_ = parsed_data["state"];

        // 判断是否操作成功
        if (state_ == SUCCESS)
        {
            cout << "操作成功！" << endl;
        }
        else if (state_ == FAIL)
        {
            cout << "不存在此拉黑好友！" << endl;
        }
        else if (state_ == USERNAMEUNEXIST)
        {
            cout << "查无此人! " << endl;
        }
    }

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
    return;
}

// 好友聊天历史记录
string historychat_client(int client_socket, string id, Queue<string> &RecvQue, int fl)
{
    // 先打印出好友信息
    string re = friendinfo_client(client_socket, id, RecvQue, 0);
    string str;

    if (re != "fail")
    {
        string opponame;
        cout << "输入好友昵称：" << endl;
        opponame = getInputWithoutCtrlD();

        // 发送数据
        nlohmann::json sendJson_client = {
            {"id", id},
            {"opponame", opponame},
            {"flag", HISTORYCHAT},
        };
        string sendJson_client_string = sendJson_client.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // 从消息队列里取消息
        string buf = RecvQue.remove();
        json parsed_data = json::parse(buf);
        vector<string> historychat_Vector = parsed_data["vector"];
        int state_ = parsed_data["state"];

        if (state_ == FAIL)
        {
            cout << "对方不是你的好友！" << endl;
            str = "fail";
        }
        else if (state_ == USERNAMEUNEXIST)
        {
            cout << "账号不存在！" << endl;
            str = "fail";
        }
        else if (historychat_Vector.empty())
        {
            cout << "———————————————暂无历史消息——————————————" << endl;
            str = opponame;
        }
        else
        {
            // 循环打印输出
            cout << "————————————————————————————————————————————————" << endl;
            for (int i = historychat_Vector.size() - 1; i >= 0; i--)
            {
                json parsed_data = json::parse(historychat_Vector[i]); // 容器里的元素还是json类型的
                struct Chatinfo chatinfo;
                chatinfo.name = parsed_data["name"];
                chatinfo.msg = parsed_data["msg"];
                // chatinfo.time = parsed_data["time"]; // 消息发送的时间最后来

                cout << chatinfo.name << ": " << chatinfo.msg << endl;
            }
            cout << "————————————————以上为历史消息———————————————————" << endl;
            str = opponame;
        }
    }

    if (fl == 1 || re == "fail")
    {
        cout << "按'q'返回上一级" << endl;
        string a;
        while ((a = getInputWithoutCtrlD()) != "q")
        {
        }
        str = "fail";
    }

    return str; // 返回所选择的好友昵称
}

// 与好友聊天
void chatfriend_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 先打印出历史消息（包括选择好友）
    string opponame = historychat_client(client_socket, id, RecvQue, 0);
    if (opponame != "fail") // 说明该用户不存在
    {

        chatname = opponame;

        string msg;
        cout << "(开始聊天吧，按'Esc'键退出)" << endl;
        while ((msg = getInputWithoutCtrlD()) != "esc") // 按'Esc'键退出聊天
        {
            if (msg == "")
                continue;
            // 发送数据
            nlohmann::json sendJson_client = {
                {"id", id},
                {"opponame", opponame},
                {"flag", CHATFRIEND},
                {"msg", msg},
            };
            string sendJson_client_string = sendJson_client.dump();
            SendMsg sendmsg;
            sendmsg.SendMsg_client(client_socket, sendJson_client_string);

            // 从消息队列里取消息
            string buf = RecvQue.remove();
            json parsed_data = json::parse(buf);
            int state_ = parsed_data["state"];
            if (state_ == FAIL)
            {
                cout << "消息发送失败,请检查是否屏蔽对方 或 已被对方屏蔽" << endl;
                cout << "按'Esc'键退出聊天" << endl;
            }
        }
        chatname = "";
    }
    else
    {
        cout << "按'q'返回上一级" << endl;
        string a;
        while ((a = getInputWithoutCtrlD()) != "q")
        {
        }
        return;
    }
}

// 个人信息
void personalinfo_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 发送数据
    nlohmann::json sendJson_client = {
        {"id", id},
        {"flag", PERSONALINFO},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    User user;
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    user.username = parsed_data["username"];
    user.password = parsed_data["password"];
    user.secrecy = parsed_data["secrecy"];

    // 打印个人信息
    cout << "用户名：" << user.username << endl;
    cout << "id：" << id << endl;
    cout << "密码：***" << endl;
    cout << "密保：*** " << endl;

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
    return;
}

// 群聊
void group_client(int client_socket, string id, Queue<string> &RecvQue)
{
    system("clear");
    groupmenuUI();

    int num_ = 1;
    do
    {
        // 清空缓冲区
        std::cin.clear();
        std::cin.sync();

        string str = getInputWithoutCtrlD();
        num_ = checkcin(str);

        switch (num_)
        {
        case 1:
            system("clear");
            creatgroup_client(client_socket, id, RecvQue);
            groupmenuUI();
            break;
        case 2:
            system("clear");
            addgroup_client(client_socket, id, RecvQue);
            // system("clear");
            groupmenuUI();
            break;
        case 3:
            system("clear");
            checkgroup_client(client_socket, id, RecvQue, 1);
            system("clear");
            groupmenuUI();
            break;
        case 4:
            system("clear");
            outgroup_client(client_socket, id, RecvQue);
            groupmenuUI();
            break;
        case 5:
            system("clear");
            checkgroupnum_client(client_socket, id, RecvQue, 1);
            system("clear");
            groupmenuUI();
            break;
        case 6:
            system("clear");
            managegroup_client(client_socket, id, RecvQue);
            system("clear");
            groupmenuUI();
            break;
        case 7:
            system("clear");
            groupchat_client(client_socket, id, RecvQue);
            system("clear");
            groupmenuUI();
            break;
        case 8:
            system("clear");
            historygroupchat_client(client_socket, id, RecvQue, 1);
            system("clear");
            groupmenuUI();
            break;
        case 0:
            system("clear");
            groupmenuUI();
            break;
        default:
            cout << "无效的数字，请重新输入！" << endl;
            break;
        }
    } while (num_ != 9); // 退出循环，返回上一级

    system("clear");
    return;
}

// 文件传输
void file_client(int client_socket, string id, Queue<string> &RecvQue)
{
    system("clear");
    filemenuUI();

    int num_ = 1;
    do
    {
        // 清空缓冲区
        std::cin.clear();
        std::cin.sync();

        string str = getInputWithoutCtrlD();
        num_ = checkcin(str);

        switch (num_)
        {
        case 1:
            system("clear");
            sendfile_client(client_socket, id, RecvQue);
            filemenuUI();
            break;
        case 2:
            system("clear");
            recvfile_client(client_socket, id, RecvQue);
            // system("clear");
            filemenuUI();
            break;
        }
    } while (num_ != 3); // 退出循环，返回上一级

    system("clear");
    return;
}

#endif