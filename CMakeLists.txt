# 设置项目名称
project(chatroom)

# 设置要求的最低 CMake 版本
cmake_minimum_required(VERSION 3.10)

include_directories(
    server
    client
    others
)

# ————————只要cc，不用.h
# 添加可执行文件 "client" 并指定源文件
add_executable(client 
    client/client.cc 
    client/menu.cc
    others/data.cc
    others/define.h 
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
    others/threadpool.h 
    others/threadpool.cc 
    others/IO.h 
    others/IO.cc 
    others/data.cc
    others/define.h 
    server/login.hpp
    others/redis.h
    others/redis.cc
    server/personalprocess.hpp
    server/groupprocess.hpp
    server/managegroupprocess.hpp
    server/fileprocess.h
    server/fileprocess.cc
)

# 添加链接库
find_library(HIREDIS_LIBRARY NAMES hiredis)
target_link_libraries(server ${HIREDIS_LIBRARY})
target_link_libraries(client ${HIREDIS_LIBRARY})
