# 设置项目名称
project(chatroom)

# 设置要求的最低 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 在这些文件夹里找头文件
include_directories(
    server
    client
    others
)

# ———————— 只要cc，不用.h ————————
# 添加可执行文件 "client" 并指定源文件
add_executable(client 
    client/client.cc 
    client/menu.cc
    others/data.cc
    others/IO.cc
    others/threadwork.cc
    others/queue.hpp
    client/groupmenu.cc
    client/managegroupmenu.cc
    client/filemenu.cc
    client/personalmenu.cc
)

# 添加可执行文件 "server" 并指定源文件
add_executable(server 
    server/server.cc 
    others/threadpool.cc 
    others/IO.cc 
    others/data.cc
    server/login.cc
    others/redis.cc
    server/personalprocess.cc
    server/groupprocess.cc
    server/managegroupprocess.cc
    server/fileprocess.cc
)

# 添加链接库
find_library(HIREDIS_LIBRARY NAMES hiredis)
target_link_libraries(server ${HIREDIS_LIBRARY})
target_link_libraries(client ${HIREDIS_LIBRARY})
