// 客户端的群聊选项界面，及与群聊有关的事务
#ifndef GROUPMENU_HPP
#define GROUPMENU_HPP

#include "../others/data.h"
#include "../others/define.h"
#include "../others/head.h"
#include "../others/IO.h"
#include "menu.hpp"
#include "../others/threadwork.hpp"
#include "managegroupmenu.hpp"

#include <iostream>
using json = nlohmann::json;
using namespace std;

void groupmenuUI(void)
{
    cout << "——————————————————————————————————————————————————" << endl;
    cout << "----------------------  群聊  ---------------------" << endl;
    cout << "——————————————————————————————————————————————————" << endl;
    cout << "                      1.创建群组                   " << endl;
    cout << "                      2.加入群组                   " << endl;
    cout << "                      3.查看已加入的群组            " << endl;
    cout << "                      4.退出已加入的群组            " << endl;
    cout << "                      5.查看群组成员列表            " << endl;
    cout << "                      6.管理群组（群主、管理员）     " << endl;
    cout << "                      7.选择群组聊天               " << endl;
    cout << "                      8.查看群组聊天记录            " << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "                      9.返回上一级                 " << endl;
    cout << "---------------------------------------------------" << endl;
    cout << "                      0.刷新页面                   " << endl;
    cout << "———————————————————————————————————————————————————" << endl;
}

// 创建群组
void creatgroup_client(int client_socket, string id, Queue<string> &RecvQue)
{
    Group group;
    cout << "输入你要创建的群组名称：(不可有空格或其他非法字符)" << endl;
    group.groupname = getInputWithoutCtrlD();
    group.ownerid = id;
    group.flag = CREATGROUP;

    // 发送数据
    nlohmann::json sendJson_client = {
        {"groupname", group.groupname},
        {"flag", group.flag},
        {"ownerid", group.ownerid},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    int state_ = parsed_data["state"];
    string groupid = parsed_data["groupid"];

    // 判断是否创建成功
    if (state_ == SUCCESS)
    {
        cout << "创建成功！ 群id为：" << groupid << endl;
    }
    else if (state_ == USERNAMEEXIST)
    {
        cout << "该群组名称已存在，请更改名称后重新创建！" << endl;
    }
    else if (state_ == FAIL)
    {
        cout << "创建失败！" << endl;
        return;
    }
}

// 加入群组
void addgroup_client(int client_socket, string id, Queue<string> &RecvQue)
{
    Group group;

    cout << "请输入你要加入的群组ID：";
    group.groupid = getInputWithoutCtrlD();
    group.userid = id;
    group.flag = ADDGROUP;

    // 发送数据
    nlohmann::json sendJson_client = {
        {"groupid", group.groupid},
        {"userid", group.userid},
        {"flag", group.flag},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    int state_ = parsed_data["state"];

    // 判断是否发送成功
    if (state_ == HADINGROUP)
    {
        cout << "你已加入该群！" << endl;
    }
    else if (state_ == USERNAMEUNEXIST)
    {
        cout << "该群id不存在！" << endl;
    }
    else if (state_ == SUCCESS)
    {
        cout << "已发送加群申请！" << endl;
    }

    return;
}

// 查看已加入的群组
int checkgroup_client(int client_socket, string id, Queue<string> &RecvQue, int fl)
{
    int re;
    // 发送数据
    nlohmann::json sendJson_client = {
        {"userid", id},
        {"flag", CHECKGROUP},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    vector<string> groupname_Vector = parsed_data["groupnamevector"];
    vector<string> groupid_Vector = parsed_data["groupidvector"];
    vector<int> groupposition_Vector = parsed_data["grouppositionvector"];

    if (groupname_Vector.empty())
    {
        cout << "暂无加入任何群组！" << endl;
        re = 0;
    }
    else
    {
        // 循环打印输出
        cout << "————————————以下为已加入的群组列表————————————" << endl;
        for (int i = 0; i < groupname_Vector.size(); i++)
        {
            cout << groupname_Vector[i] << "  " << groupid_Vector[i];
            if (groupposition_Vector[i] == 2)
            {
                cout << "（群主）" << endl;
            }
            else if (groupposition_Vector[i] == 1)
            {
                cout << "（管理员）" << endl;
            }
            else
            {
                cout << '\n';
            }
        }
        cout << "——————————————————————————————————————————" << endl;
        re = 1;
    }

    if (fl == 1)
    {
        cout << "按'q'返回上一级" << endl;
        string a;
        while ((a = getInputWithoutCtrlD()) != "q")
        {
        }
        return re;
    }

    return re;
}

// 退出已加入的群组
void outgroup_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 先打印出群聊信息
    int re = checkgroup_client(client_socket, id, RecvQue, 0);
    if (re == 0) // 没加入任何群组，则直接返回
        return;

    Group group;
    cout << "请输入你要退出的群id：（警告：若你为该群的群主，则直接解散该群！）";
    group.groupid = getInputWithoutCtrlD();
    group.userid = id;
    group.flag = OUTGROUP;

    // 发送数据
    nlohmann::json sendJson_client = {
        {"groupid", group.groupid},
        {"userid", group.userid},
        {"flag", group.flag},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    int state_ = parsed_data["state"];

    // 判断是否退出成功
    if (state_ == SUCCESS)
    {
        cout << "退出成功！" << endl;
    }
    else if (state_ == FAIL)
    {
        cout << "退出失败！" << endl;
    }
}

// 查看群组成员列表
string checkgroupnum_client(int client_socket, string id, Queue<string> &RecvQue, int fl)
{
    // 先打印出群聊信息
    int re = checkgroup_client(client_socket, id, RecvQue, 0);
    if (re == 0)
        return "fail";

    Group group;
    cout << "请输入你要查看的群id：";
    group.groupid = getInputWithoutCtrlD();
    group.userid = id;
    group.flag = CHECKGROUPNUM;

    // 发送数据
    nlohmann::json sendJson_client = {
        {"userid", group.userid},
        {"groupid", group.groupid},
        {"flag", group.flag},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 从消息队列里取消息
    string buf = RecvQue.remove();
    json parsed_data = json::parse(buf);
    vector<string> groupnumname_Vector = parsed_data["groupnumnamevector"];
    vector<string> groupnumid_Vector = parsed_data["groupnumidvector"];
    vector<int> groupnumposition_Vector = parsed_data["groupnumpositionvector"];
    int state_ = parsed_data["state"];

    // 判断是否成功
    if (state_ == USERNAMEUNEXIST)
    {
        cout << "该群id不存在！" << endl;
    }
    else if (state_ == FAIL)
    {
        cout << "你未加入该群，无法查看！" << endl;
    }
    else
    {
        // 循环打印输出
        cout << "————————————以下为群组成员列表————————————" << endl;
        for (int i = 0; i < groupnumname_Vector.size(); i++)
        {
            cout << groupnumname_Vector[i] << "  " << groupnumid_Vector[i];
            if (groupnumposition_Vector[i] == 2)
            {
                cout << "（群主）" << endl;
            }
            else if (groupnumposition_Vector[i] == 1)
            {
                cout << "（管理员）" << endl;
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

    if (state_ == SUCCESS)
        return group.groupid;
    else
        return "fail";
}

// 管理群组
void managegroup_client(int client_socket, string id, Queue<string> &RecvQue)
{
    system("clear");
    manegegroupUI();

    int num__ = 1;
    do
    {
        // 清空缓冲区
        std::cin.clear();
        std::cin.sync();

        string str = getInputWithoutCtrlD();
        num__ = checkcin(str);

        switch (num__)
        {
        case 1:
            system("clear");
            addmin_client(client_socket, id, RecvQue);
            manegegroupUI();
            break;
        case 2:
            system("clear");
            deladmin_client(client_socket, id, RecvQue);
            manegegroupUI();
            break;
        case 3:
            system("clear");
            checkapplylist_client(client_socket, id, RecvQue, 1);
            system("clear");
            manegegroupUI();
            break;
        case 4:
            system("clear");
            delgroupnum_client(client_socket, id, RecvQue);
            system("clear");
            manegegroupUI();
            break;
        case 5:
            system("clear");
            delgroup_client(client_socket, id, RecvQue);
            system("clear");
            manegegroupUI();
            break;
        case 0:
            system("clear");
            manegegroupUI();
            break;
        default:
            cout << "无效的数字，请重新输入！" << endl;
            break;
        }
    } while (num__ != 6); // 退出循环，返回上一级

    system("clear");
    return;
}

// 查看群组聊天记录
string historygroupchat_client(int client_socket, string id, Queue<string> &RecvQue, int fl)
{
    // 先打印出群聊信息
    int re = checkgroup_client(client_socket, id, RecvQue, 0);
    if (re != 0)
    {
        Group group;
        cout << "请输入你要查看聊天记录的群id：";
        group.groupid = getInputWithoutCtrlD();
        group.userid = id;
        group.flag = HISTORYGROUPCHAT;

        // 发送数据
        nlohmann::json sendJson_client = {
            {"userid", group.userid},
            {"groupid", group.groupid},
            {"flag", group.flag},
        };
        string sendJson_client_string = sendJson_client.dump();
        SendMsg sendmsg;
        sendmsg.SendMsg_client(client_socket, sendJson_client_string);

        // 从消息队列里取消息
        string buf = RecvQue.remove();
        json parsed_data = json::parse(buf);
        vector<string> historygroupchat_Vector = parsed_data["vector"];
        int state_ = parsed_data["state"];
        string str;

        if (state_ == FAIL)
        {
            cout << "历史消息获取失败！请检查群id是否正确" << endl;
            str = "fail";
        }
        else if (historygroupchat_Vector.empty())
        {
            cout << "———————————————暂无历史消息——————————————" << endl;
            str = group.groupid;
        }
        else
        {
            // 循环打印输出
            cout << "————————————————————————————————————————————————" << endl;
            for (int i = historygroupchat_Vector.size() - 1; i >= 0; i--)
            {
                json parsed_data = json::parse(historygroupchat_Vector[i]); // 容器里的元素还是json类型的
                struct Chatinfo chatinfo;
                chatinfo.name = parsed_data["name"];
                chatinfo.msg = parsed_data["msg"];
                // chatinfo.time = parsed_data["time"]; // 消息发送的时间最后来

                cout << chatinfo.name << ": " << chatinfo.msg << endl;
            }
            cout << "————————————————以上为历史消息———————————————————" << endl;
            str = group.groupid;
        }

        if (fl == 1)
        {
            cout << "按'q'返回上一级" << endl;
            string a;
            while ((a = getInputWithoutCtrlD()) != "q")
                str = "";
        }

        return str; // 返回所选择的群聊id
    }

    cout << "按'q'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "q")
    {
    }
    return "";
}

// 选择群组聊天
void groupchat_client(int client_socket, string id, Queue<string> &RecvQue)
{
    Group group;
    // 先打印出历史消息（包括选择群聊）
    group.groupid = historygroupchat_client(client_socket, id, RecvQue, 0);
    if (group.groupid != "fail" && group.groupid != "") // 说明该群id存在
    {

        chatgroup = group.groupid;

        group.msg;
        group.userid = id;
        group.flag = GROUPCHAT;
        cout << "(开始聊天吧，按'Esc'键退出)" << endl;
        while ((group.msg = getInputWithoutCtrlD()) != "esc") // 按'Esc'键退出聊天
        {
            if (group.msg == "")
                continue;
                
            // 发送数据
            nlohmann::json sendJson_client = {
                {"groupid", group.groupid},
                {"userid", group.userid},
                {"flag", group.flag},
                {"msg", group.msg},
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
                cout << "消息发送失败,请检查是否 你已被移除群聊 或 该群聊已被解散" << endl;
                cout << "按'Esc'键退出聊天" << endl;
            }
        }

        chatgroup = "";
    }
    else if (group.groupid != "")
    {
        cout << "按'q'返回上一级" << endl;
        string a;
        while ((a = getInputWithoutCtrlD()) != "q")
        {
        }
    }
}

#endif
