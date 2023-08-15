# 设置项目名称
project(chatroom)

# 设置要求的最低 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 添加可执行文件 "client" 并指定源文件
add_executable(client 
    client/client.cc 
    client/menu.hpp 
    others/data.h 
    others/define.h 
    others/head.h 
    others/IO.h 
    client/personalmenu.hpp 
    others/IO.cc
    others/threadwork.hpp
    others/queue.hpp
    client/groupmenu.hpp
    client/managegroupmenu.hpp
)

# 添加可执行文件 "server" 并指定源文件
add_executable(server 
    server/server.cc 
    others/threadpool.h 
    others/threadpool.cc 
    others/IO.h 
    others/IO.cc 
    others/data.h 
    others/define.h 
    server/login.hpp
    others/redis.hpp
    server/personalprocess.hpp
    server/groupprocess.hpp
    server/managegroupprocess.hpp
)

# 添加链接库
find_library(HIREDIS_LIBRARY NAMES hiredis)
target_link_libraries(server ${HIREDIS_LIBRARY})
target_link_libraries(client ${HIREDIS_LIBRARY})
