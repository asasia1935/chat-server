cmake_minimum_required(VERSION 3.10)
project(ChatServer)

set(CMAKE_CXX_STANDARD 17)

# include 디렉토리 추가
include_directories(include)

# level1: 단순 echo 서버
add_executable(level1_echo_server
    src/level1-echo/echo_server.cpp
)

# level2: select 기반 (Socket 클래스 사용)
add_executable(level2_select_server
    src/level2-select/select_server.cpp
    src/common/Socket.cpp
)