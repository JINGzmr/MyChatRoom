# 设置项目名称
project(chatroom)

# 设置要求的最低 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 添加可执行文件 "client" 并指定源文件
add_executable(client 
    client.cc 
    menu.hpp 
    data.h 
    define.h 
    head.h 
    IO.h 
    personalmenu.hpp 
    IO.cc
    threadwork.hpp
    queue.hpp
    groupmenu.hpp
    managegroupmenu.hpp
)

# 添加可执行文件 "server" 并指定源文件
add_executable(server 
    server.cc 
    threadpool.h 
    threadpool.cc 
    IO.h 
    IO.cc 
    data.h 
    define.h 
    login.hpp
    redis.hpp
    personalprocess.hpp
    groupprocess.hpp
    managegroupprocess.hpp
)

# 添加链接库
find_library(HIREDIS_LIBRARY NAMES hiredis)
target_link_libraries(server ${HIREDIS_LIBRARY})
target_link_libraries(client ${HIREDIS_LIBRARY})
