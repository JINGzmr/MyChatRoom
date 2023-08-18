#include "managegroupmenu.h"

void manegegroupUI(void)
{
    cout << "——————————————————————————————————————————————————" << endl;
    cout << "---------------------  管理群组  -------------------" << endl;
    cout << "——————————————————————————————————————————————————" << endl;
    cout << "                      1.添加管理员（群主）          " << endl;
    cout << "                      2.删除管理员（群主）          " << endl;
    cout << "                      3.编辑加群申请               " << endl;
    cout << "                      4.删除群成员                 " << endl;
    cout << "                      5.解散该群（群主）            " << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "                      0.刷新页面                   " << endl;
    cout << "———————————————————————————————————————————————————" << endl;
}

// 添加管理员
void addmin_client(int client_socket, string id, Queue<string> &RecvQue)
{

    // 先打查看群成员列表
    string re = checkgroupnum_client(client_socket, id, RecvQue, 0);
    if (re == "fail")
    {
        return;
    }
    else
    {
        Group group;
        group.groupid = re;
        cout << "请输入你要设置管理员的成员id：（警告：此操作仅对群主生效！）";
        group.oppoid = getInputWithoutCtrlD();
        group.userid = id;
        group.flag = ADDADMIN;

        // 发送数据
        nlohmann::json sendJson_client = {
            {"oppoid", group.oppoid},
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
        int state_ = parsed_data["state"];

        // 判断是否成功
        if (state_ == USERNAMEUNEXIST)
        {
            cout << "该用户不存在!" << endl;
        }
        else if (state_ == NOTINGROUP)
        {
            cout << "该用户不在群聊中!" << endl;
        }
        else if (state_ == SUCCESS)
        {
            cout << "群管理添加成功！" << endl;
        }
        else if (state_ == HADADMIN)
        {
            cout << "该成员已是管理员或群主！" << endl;
        }
        else if (state_ == FAIL)
        {
            cout << "你权限不够，操作失败！" << endl;
        }
    }
}

// 删除群管理员
void deladmin_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 先打查看群成员列表
    string re = checkgroupnum_client(client_socket, id, RecvQue, 0);
    if (re == "fail")
    {
        return;
    }
    else
    {
        Group group;
        group.groupid = re;
        cout << "请输入你要删除管理员的成员id：（警告：此操作仅对群主生效！）";
        group.oppoid = getInputWithoutCtrlD();
        group.userid = id;
        group.flag = DELADMIN;

        // 发送数据
        nlohmann::json sendJson_client = {
            {"oppoid", group.oppoid},
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
        int state_ = parsed_data["state"];

        // 判断是否成功
        if (state_ == USERNAMEUNEXIST)
        {
            cout << "该用户不存在!" << endl;
        }
        else if (state_ == NOTINGROUP)
        {
            cout << "该用户不在群聊中!" << endl;
        }
        else if (state_ == SUCCESS)
        {
            cout << "群管理删除成功！" << endl;
        }
        else if (state_ == NOTADMIN)
        {
            cout << "该成员不是管理员！" << endl;
        }
        else if (state_ == FAIL)
        {
            cout << "你权限不够，操作失败！" << endl;
        }
    }
}

// 查看申请加群列表(包括同意请求一起写，但服务器那边要有不同的函数来处理)
void checkapplylist_client(int client_socket, string id, Queue<string> &RecvQue, int fl)
{
    // 先打印出群聊信息
    int re = checkgroup_client(client_socket, id, RecvQue, 0);
    if (re != 0) // 表明该用户有加入的群聊
    {
        Group group;
        cout << "请输入你要查看的群id：（警告：此操作仅对群主、管理员生效！）";
        group.groupid = getInputWithoutCtrlD();
        group.userid = id;
        group.flag = CHECKAPPLYLIST;

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
        vector<string> groupapply_Vector = parsed_data["groupapplyvector"];
        int state_ = parsed_data["state"];

        // 判断是否成功
        if (state_ == USERNAMEUNEXIST)
        {
            cout << "该群id不存在！" << endl;
        }
        else if (state_ == FAIL)
        {
            cout << "你未加入该群 或 没有权限，无法查看！" << endl;
        }
        else
        {
            if (groupapply_Vector.empty())
            {
                cout << "暂无加群申请！" << endl;
            }
            else
            {
                // 循环打印输出
                cout << "————————————以下为群组申请列表————————————" << endl;
                for (int i = 0; i < groupapply_Vector.size(); i++)
                {
                    cout << groupapply_Vector[i] << endl;
                }
                cout << "——————————————————————————————————————————" << endl;

                // 同意加群申请
                string state;
                cout << "输入要编辑的用户昵称" << endl;
                group.opponame = getInputWithoutCtrlD();
                cout << "同意---1 / 拒绝---0" << endl;
                state = getInputWithoutCtrlD();

                // 发送数据
                nlohmann::json sendJson_client = {
                    {"userid", group.userid},
                    {"groupid", group.groupid},
                    {"opponame", group.opponame},
                    {"state", state},
                    {"flag", AGREEAPPLY},
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
        }
    }

    if (fl == 1)
    {
        cout << "按'Esc'返回上一级" << endl;
        string a;
        while ((a = getInputWithoutCtrlD()) != "esc")
        {
        }
        return;
    }
}

// 删除群成员
void delgroupnum_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 先打查看群成员列表
    string re = checkgroupnum_client(client_socket, id, RecvQue, 0);
    if (re != "fail")
    {
        Group group;
        group.groupid = re;
        do
        {
            cout << "请输入你要删除的成员id：（警告：此操作仅对群主、管理员生效！）";
            group.oppoid = getInputWithoutCtrlD();
            group.userid = id;
            group.flag = DELGROUPNUM;
            if (group.userid == group.oppoid)
            {
                cout << "不可删除自己！请重新输入！" << endl;
            }
        } while (group.userid == group.oppoid);

        // 发送数据
        nlohmann::json sendJson_client = {
            {"oppoid", group.oppoid},
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
        int state_ = parsed_data["state"];

        // 判断是否成功
        if (state_ == USERNAMEUNEXIST)
        {
            cout << "该用户不存在!" << endl;
        }
        else if (state_ == NOTINGROUP)
        {
            cout << "该用户不在群聊中!" << endl;
        }
        else if (state_ == SUCCESS)
        {
            cout << "该用户删除成功！" << endl;
        }
        else if (state_ == FAIL) //**群主可以踢任何人，但管理员不能踢管理员和群主**
        {
            cout << "你权限不够，操作失败！" << endl;
        }
    }

    cout << "按'Esc'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "esc")
    {
    }
}

// 解散群组
void delgroup_client(int client_socket, string id, Queue<string> &RecvQue)
{
    // 先打印出群聊信息
    int re = checkgroup_client(client_socket, id, RecvQue, 0);
    if (re != 0) // 没加入任何群组，则直接返回
    {

        Group group;
        cout << "请输入你要解散的群id：（警告：此操作仅对群主生效！）";
        group.groupid = getInputWithoutCtrlD();
        group.userid = id;
        group.flag = DELGROUP;

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
        int state_ = parsed_data["state"];//解析有问题

        // 判断是否成功
        if (state_ == SUCCESS)
        {
            cout << "解散成功！" << endl;
        }
        else if (state_ == FAIL)
        {
            cout << "权限不够！" << endl;
        }
        // else
        // {
        //     cout << "解散失败！请检查id是否正确" << endl;
        // }
    }

    cout << "按'Esc'返回上一级" << endl;
    string a;
    while ((a = getInputWithoutCtrlD()) != "esc")
    {
    }
}