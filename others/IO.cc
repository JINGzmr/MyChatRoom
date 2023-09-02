#include "../others/IO.h"
#include "head.h"
#include "../others/define.h"
//************——————>后期要改
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