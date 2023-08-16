#include "../others/IO.h"
#include "head.h"
#include "../others/define.h"
//************——————》后期要改
int SendMsg::writen(int fd, char *msg, int size)
{
  char *buf = msg;
  int count = size;
  while (count > 0)
  {
    int len = send(fd, buf, count, 0);
    if (len <= -1)
    {

      if (len == -1 && errno == EINTR)
        continue;
      else
        return -1;
    }
    else if (len == 0)
    {
      continue;
    }
    buf += len;
    count -= len;
  }
  return size;
}

// 客户端发送序列化好的数据
void SendMsg::SendMsg_client(int client_socket, const string &str)
{
  // int DataSize = htonl(str.length()); // 发给服务端的包的大小

  // if (send(client_socket, &DataSize, sizeof(int), 0) == -1)
  // {
  //     std::cout << "client send data size failed" << std::endl;
  // }
  // else
  // {
  //     std::cout << "client data sent to the server" << std::endl;
  // }

  // if (send(client_socket, str.c_str(), str.size(), 0) == -1)
  // {
  //     std::cout << "client send failed" << std::endl;
  // }
  // else
  // {
  //     std::cout << "client data sent to the server" << std::endl;
  // }

  if (client_socket < 0 || str.c_str() == NULL || str.size() <= 0)
  {
    return;
  }
  char *data = (char *)malloc(sizeof(char) * (str.size() + 4));
  int biglen = htonl(str.size());
  memcpy(data, &biglen, 4);
  memcpy(data + 4, str.c_str(), str.size());
  int ret;
  ret = writen(client_socket, data, str.size() + 4);
  if (ret == -1)
  {
    perror("send error");
    close(client_socket);
  }
}

// // 服务端发送序列化好的数据
// void SendMsg::SendMsg_server(int client_socket, const std::string &str)
// {
// int DataSize = htonl(str.length()); // 发给服务端的包的大小

// if (send(client_socket, &DataSize, sizeof(int), 0) == -1)
// {
//     std::cout << "server send data size failed" << std::endl;
// }
// else
// {
//     std::cout << "server data sent to the server" << std::endl;
// }

// if (send(client_socket, str.c_str(), str.size(), 0) == -1)
// {
//     std::cout << "server send failed" << std::endl;
// }
// else
// {
//     std::cout << "server data sent to the server" << std::endl;
// }

// }

// 服务端发送数据处理的结果（成功/失败）
void SendMsg::SendMsg_int(int client_socket, int state)
{
  if (send(client_socket, &state, sizeof(int), 0) == -1)
  {
    std::cout << "state send failed" << std::endl;
  }
  else
  {
    std::cout << "state sent success" << std::endl;
  }
}

/*---------------------------------------------------------------------------*/
int RecvMsg::readn(int fd, char *buf, int size)
{
  char *pt = buf;
  int count = size;
  while (count > 0)
  {
    int len = recv(fd, pt, count, 0);
    if (len == -1)
    {
      if (errno == EINTR || errno == EWOULDBLOCK)
        continue;
      else
        return -1;
    }
    else if (len == 0)
    {
      return size - count;
    }
    pt += len;
    count -= len;
  }
  return size - count;
}

// 客户端接收序列化的数据
int RecvMsg::RecvMsg_client(int client_socket, string &str)
{
  // int DataSize = 0; // 从服务端发送过来的包的大小
  // char buffer[BUFFER_SIZE];
  // memset(buffer, 0, BUFFER_SIZE);

  // ssize_t recv_bytes = recv(client_socket, &DataSize, sizeof(int), 0);
  // if (recv_bytes == -1)
  // {
  //     std::cout << "client recv data size failed" << std::endl;
  // }
  // else if (recv_bytes == 0) // 客户端断开连接
  // {
  //     std::cout << "Connection closed by peer." << std::endl;
  //     close(client_socket); // 先关闭该客户端
  //     // epoll_ctl(epld, EPOLL_CTL_DEL, client_socket, 0); // 从epoll里面删除，不需要再检测了
  //     // clients.earse(client_socket); // 同时把他从map里面删除
  // }

  // recv_bytes = recv(client_socket, buffer, DataSize, 0);
  // if (recv_bytes == -1)
  // {
  //     std::cout << "client recv failed" << std::endl;
  // }
  // else if (recv_bytes == 0) // 客户端断开连接
  // {
  //     std::cout << "Connection closed by peer." << std::endl;
  //     close(client_socket); // 先关闭该客户端
  //     // epoll_ctl(epld, EPOLL_CTL_DEL, client_socket, 0); // 从epoll里面删除，不需要再检测了
  //     // clients.earse(client_socket); // 同时把他从map里面删除
  // }

  // str = string(buffer, recv_bytes); // 将字符数组buffer中的recv_bytes个字符复制到字符串str中

  int len = 0;
  readn(client_socket, (char *)&len, 4);
  len = ntohl(len);
  char *data = (char *)malloc(len + 1);
  int Len = readn(client_socket, data, len);
  if (Len == 0)
  {
    printf("对方断开链接\n");
    return -1;
  }
  else if (len != Len)
  {
    printf("数据接收失败\n");
  }
  data[len] = '\0';
  str = data;

  return Len;
}

// // 服务端接收序列化的数据
// void RecvMsg::RecvMsg_server(int client_socket, std::string &str)
// {
//     int DataSize = 0; // 从服务端发送过来的包的大小
//     char buffer[BUFFER_SIZE];
//     memset(buffer, 0, BUFFER_SIZE);

//     ssize_t recv_bytes = recv(client_socket, &DataSize, sizeof(int), 0);
//     if (recv_bytes == -1)
//     {
//         std::cout << "server recv data size failed" << std::endl;
//     }
//     else if (recv_bytes == 0) // 客户端断开连接
//     {
//         std::cout << "Connection closed by peer." << std::endl;
//         close(client_socket); // 先关闭该客户端
//         // epoll_ctl(epld, EPOLL_CTL_DEL, client_socket, 0); // 从epoll里面删除，不需要再检测了
//         // clients.earse(client_socket); // 同时把他从map里面删除
//     }

//     recv_bytes = recv(client_socket, buffer, DataSize, 0);
//     if (recv_bytes == -1)
//     {
//         std::cout << "server recv failed" << std::endl;
//     }
//     else if (recv_bytes == 0) // 客户端断开连接
//     {
//         std::cout << "Connection closed by peer." << std::endl;
//         close(client_socket); // 先关闭该客户端
//         // epoll_ctl(epld, EPOLL_CTL_DEL, client_socket, 0); // 从epoll里面删除，不需要再检测了
//         // clients.earse(client_socket); // 同时把他从map里面删除
//     }

//     str = string(buffer, recv_bytes); // 将字符数组buffer中的recv_bytes个字符复制到字符串str中
// }

// 客户端接收数据处理的结果（成功/失败）
int RecvMsg::RecvMsg_int(int client_socket)
{
  int state;
  ssize_t recv_bytes = recv(client_socket, &state, sizeof(int), 0);
  if (recv_bytes == -1)
  {
    std::cout << "recv state failed" << std::endl;
  }
  else if (recv_bytes == 0) // 客户端断开连接
  {
    std::cout << "Connection closed by peer." << std::endl;
    close(client_socket);
  }
  return state;
}