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

# level 3: poll 기반 (Socket 클래스 사용)
add_executable(level3_poll_server
    src/level3-poll/poll_server.cpp
    src/common/Socket.cpp
)

# level4: epoll LT 기반
add_executable(level4_epoll_lt_server
    src/level4-epoll-lt/epoll_lt_server.cpp
    src/common/Socket.cpp
)

# level5: epoll ET 기반
add_executable(level5_epoll_et_server
    src/level5-epoll-et/epoll_et_server.cpp
    src/common/Socket.cpp
)