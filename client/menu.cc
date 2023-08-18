#include "menu.h"

void menu(int client_socket)
{
    int num;
    do
    {
        cout << "——————————————————————————————————————————————————" << endl;
        cout << "------------------欢迎进入聊天室！-------------------" << endl;
        cout << "——————————————————————————————————————————————————" << endl;
        cout << "---------------------1.登录------------------------" << endl;
        cout << "---------------------2.注册------------------------" << endl;
        cout << "---------------------3.注销------------------------" << endl;
        cout << "---------------------4.退出------------------------" << endl;
        cout << "---------------------0.找回密码---------------------" << endl;
        cout << "——————————————————————————————————————————————————" << endl;

        // 清空缓冲区
        std::cin.clear();
        std::cin.sync();

        string str = getInputWithoutCtrlD();
        num = checkcin(str);
        switch (num)
        {
        case 1:
            login_client(client_socket); // 传一个客户端的socket进来
            break;
        case 2:
            register_client(client_socket);
            break;
        case 3:
            signout_client(client_socket);
            break;
        case 4:
            out_client(client_socket);
            break;
        case 0:
            findpassword_client(client_socket);
            break;
        default:
            cout << "无效的数字，请重新输入！" << endl;
            break;
        }
    } while (num != 4);
}

void login_client(int client_socket)
{
    User user;
    cout << "请输入id: ";
    user.id = getInputWithoutCtrlD();
    cout << "请输入密码: ";
    user.password = withoutdisplay();
    user.flag = LOGIN; // 表示是要登录

    // json序列化，及向服务器发送数据
    nlohmann::json sendJson_client = {
        {"id", user.id},
        {"password", user.password},
        {"flag", user.flag},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);
 
    // 接收数据
    int state_;
    RecvMsg recvmsg;
    state_ = recvmsg.RecvMsg_int(client_socket);

    // 判断是否登入成功
    if (state_ == SUCCESS)
    {
        cout << "登入成功！" << endl;
        Queue<string> RecvQue;
        thread RecvThread = thread(recvfunc, client_socket, user.id, &RecvQue); // 工作线程启动
        //********进入下一页面的入口********
        messagemenu(client_socket, user.id, RecvQue);
        RecvThread.join(); // 回收线程
        system("clear");   // 刷新终端页面
    }
    else if (state_ == FAIL)
    {
        cout << "密码错误！" << endl;
        //*********再次回到登入界面重新输入***********
        return;
    }
    else if (state_ == ONLINE)
    {
        cout << "你已在别处登录！" << endl;
        //*********再次回到登入界面重新输入***********
        return;
    }
    else if (state_ == USERNAMEUNEXIST)
    {
        cout << "该id不存在，请注册 或 重新输入" << endl;
        //*********再次回到登入界面重新输入***********
        return;
    }
    return;
}

void register_client(int client_socket)
{
    User user;
    cout << "请输入用户名: ";
    user.username = getInputWithoutCtrlD();
    cout << "请输入密码: ";
    user.password = withoutdisplay();
    cout << "请输入密保，用于找回密码: ";
    user.secrecy = withoutdisplay();
    user.flag = REGISTER; // 表示是要注册

    // 序列化，发送数据
    nlohmann::json sendJson_client = {
        {"username", user.username},
        {"password", user.password},
        {"secrecy", user.secrecy},
        {"flag", user.flag},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 接收数据
    int state_;
    RecvMsg recvmsg;
    state_ = recvmsg.RecvMsg_int(client_socket);

    // 判断是否注册成功
    if (state_ == SUCCESS)
    {
        string id;
        recvmsg.RecvMsg_client(client_socket, id);
        cout << "注册成功！ 你的账号为：" << id << endl;
        //*******回到登入界面进行登录*********
        return;
    }
    else if (state_ == USERNAMEEXIST)
    {
        cout << "该用户名已存在，请登录 或 更改用户名后重新注册" << endl;
        //*********再次回到登入界面重新注册*************
        return;
    }
    else if (state_ == FAIL)
    {
        cout << "注册失败！" << endl;
        return;
    }
    return;
}

void signout_client(int client_socket)
{
    User user;
    cout << "请输入id: ";
    user.id = getInputWithoutCtrlD();
    cout << "请输入密码: ";
    user.password = withoutdisplay();
    user.flag = SIGNOUT; // 表示是要注销

    // 序列化，发送数据（不用把结构体的所有成员都序列化）
    nlohmann::json sendJson_client = {
        {"id", user.id},
        {"password", user.password},
        {"flag", user.flag},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 接收数据
    int state_;
    RecvMsg recvmsg;
    state_ = recvmsg.RecvMsg_int(client_socket);

    // 判断是否注销成功
    if (state_ == SUCCESS)
    {
        cout << "注销成功！" << endl;
        //*******回到登入界面，看用户是否注册登入*********
        return;
    }
    else if (state_ == USERNAMEUNEXIST)
    {
        cout << "该用户名不存在，请注册 或 重新输入" << endl;
        //*********再次回到登入界面重新注销*************
    }
    else
    {
        cout << "注销失败！---请检查密码输入是否正确" << endl;
        //*********再次回到登入界面重新注销*************
        return;
    }
}

void out_client(int client_socket)
{
    // 序列化，发送数据（不用把结构体的所有成员都序列化）
    nlohmann::json sendJson_client = {
        {"flag",OUT},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    cout << "退出成功！"<< endl;
}

void findpassword_client(int client_socket)
{
    User user;
    cout << "请输入id: ";
    user.id = getInputWithoutCtrlD();
    cout << "请输入密保：";
    user.secrecy = withoutdisplay();
    user.flag = FINDPASSWORD; // 表示是要找回密码

    // 序列化，发送数据（不用把结构体的所有成员都序列化）
    nlohmann::json sendJson_client = {
        {"id", user.id},
        {"secrecy", user.secrecy},
        {"flag", user.flag},
    };
    string sendJson_client_string = sendJson_client.dump();
    SendMsg sendmsg;
    sendmsg.SendMsg_client(client_socket, sendJson_client_string);

    // 接收数据
    int state_;
    RecvMsg recvmsg;
    state_ = recvmsg.RecvMsg_int(client_socket);

    // 判断密码是否找回成功
    if (state_ == SUCCESS)
    {
        string password;
        recvmsg.RecvMsg_client(client_socket, password);
        cout << "找回成功！ 你的密码为：" << password << endl;
        return;
    }
    else if (state_ == USERNAMEUNEXIST)
    {
        cout << "该用户名不存在，请注册 或 重新输入" << endl;
    }
    else
    {
        cout << "找回失败！---请检查密保输入是否正确" << endl;
        return;
    }
}
