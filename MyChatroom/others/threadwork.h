// 客户端的工作线程，客户端所有的recv都在这里接收
// 工作线程根据recv的标头来判断是 返回的消息 还是 通知事件
// 如果是返回的消息，则将这个消息加入到消息队列里，客户端的子线程从消息队列里取消息
// 如果是通知消息，判断是否是聊天消息，私聊还是群聊，用户是否在聊天室里

#ifndef THREADWORK_H
#define THREADWORK_H

#include "head.h"
#include "../others/queue.hpp"
#include "../others/define.h"
#include "../others/data.h"
#include "../others/IO.h"

// 根据type来决定是否将受到的recvjson_buf放入消息队列里
// type：普通normal、通知notice
// 每个收到的buf都有type、flag

void recvfunc(int fd, string id, Queue<string> *RecvQue);


#endif
