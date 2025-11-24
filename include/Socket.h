#ifndef SOCKET_H
#define SOCKET_H

#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

// RAII 소켓 관리 클래스
class SocketHandle
{
private:
    int fd_;

public:
    explicit SocketHandle(int fd);
    ~SocketHandle();

    // 복사 금지
    SocketHandle(const SocketHandle &) = delete;
    SocketHandle &operator=(const SocketHandle &) = delete;

    // 이동 허용
    SocketHandle(SocketHandle &&other) noexcept;
    SocketHandle &operator=(SocketHandle &&other) noexcept;

    int get() const { return fd_; }
    bool is_valid() const { return fd_ >= 0; }
};

// 서버 소켓 클래스
class ServerSocket
{
private:
    SocketHandle socket_;
    int port_;

public:
    explicit ServerSocket(int port);

    int get_fd() const { return socket_.get(); }
    SocketHandle accept_client();
};

#endif // SOCKET_H