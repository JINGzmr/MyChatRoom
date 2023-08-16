#ifndef DEFINE_H
#define DEFINE_H

const int BUFFER_SIZE = 10240; // 缓冲区的大小（用于存储recv来的序列化好的string）

#define NOTADMIN -12       // 不是群管理
#define NOTINGROUP -11     // 不是群成员
#define HADADMIN -10       // 已经是群管理
#define HADINGROUP -9      // 已加入群聊
#define HADBLACK -8        // 已拉黑对方
#define HADFRIEND -7       // 已添加为好友
#define USERNAMEUNEXIST -6 // 用户名不存在
#define USERNAMEEXIST -5   // 用户名已存在
#define OFFLINE -4         // 不在线
#define ONLINE -3          // 在线
#define FAIL -2            // 失败  --->不能单定义一个N为-2 ！！！，否则可能会破坏引用的三方库里的其他对N的定义
#define SUCCESS -1         // 成功
#define FINDPASSWORD 0     // 找回密码
#define LOGIN 1            // 登录请求
#define REGISTER 2         // 注册请求
#define SIGNOUT 3          // 注销请求
#define OUT 35             // 退出

#define ADDFRIEND 4         // 添加好友
#define FRIENDAPPLYLIST 51  // 查看好友申请列表
#define FRIENDAPPLYEDIT 52  // 编辑好友申请
#define CHATFRIEND 6        // 选择好友私聊
#define HISTORYCHAT 7       // 查看历史聊天记录
#define FRIENDINFO 8        // 好友信息
#define ADDBLACK 9          // 加入黑名单（屏蔽好友）
#define DELFRIEND 10        // 删除好友
#define BLACKFRIENDLIST 111 // 查看屏蔽好友列表
#define BLACKFRIENDEDIT 112 // 编辑屏蔽好友
#define PERSONALINFO 12     // 个人信息

#define CREATGROUP 17       // 创建群组
#define ADDGROUP 18         // 加入群组
#define CHECKGROUP 19       // 查看已加入的群组
#define OUTGROUP 20         // 退出已加入的群组
#define CHECKGROUPNUM 21    // 查看群组成员列表
#define MANAGEGROUP 22      // 管理群组
#define GROUPCHAT 23        // 选择群组聊天
#define HISTORYGROUPCHAT 24 // 查看群组聊天记录

#define ADDADMIN 26        // 添加管理员
#define DELADMIN 27        // 删除管理员
#define CHECKAPPLYLIST 281 // 查看申请列表
#define AGREEAPPLY 282     // 同意加群申请
#define DELGROUPNUM 29     // 删除群成员
#define DELGROUP 30        // 解散群组

#define LOGOUT 15     // 退出登录
#define REFRESH 16    // 刷新页面
#define BACK 25       // 返回上一级
#define SHOUNOTICE 50 // 展示未读消息

#define NORMAL 100  // 普通事件
#define NOTICE 200  // 通知事件
#define PRIVATE 210 // 私聊
#define GROUP 220   // 群聊

#endif