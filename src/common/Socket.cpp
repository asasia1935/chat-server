#include "Socket.h"

// ========== SocketHandle 구현 ==========

SocketHandle::SocketHandle(int fd) : fd_(fd)
{
    if (fd_ < 0)
    {
        throw std::runtime_error("잘못된 소켓 파일 디스크립터");
    }
}

SocketHandle::~SocketHandle()
{
    if (fd_ >= 0)
    {
        close(fd_);
        std::cout << "소켓 " << fd_ << " 닫힘\n";
    }
}

SocketHandle::SocketHandle(SocketHandle &&other) noexcept : fd_(other.fd_)
{
    other.fd_ = -1;
}

SocketHandle &SocketHandle::operator=(SocketHandle &&other) noexcept
{
    if (this != &other)
    {
        if (fd_ >= 0)
        {
            close(fd_);
        }
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

// ========== ServerSocket 구현 ==========

ServerSocket::ServerSocket(int port)
    : socket_(socket(AF_INET, SOCK_STREAM, 0)), port_(port)
{
    // 주소 설정
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    // 바인딩
    if (bind(socket_.get(), (sockaddr *)&address, sizeof(address)) < 0)
    {
        throw std::runtime_error("바인딩 실패");
    }

    // 리슨
    if (listen(socket_.get(), 3) < 0)
    {
        throw std::runtime_error("listen 실패");
    }

    std::cout << "서버 시작! 포트 " << port_ << "에서 대기 중...\n";
}

SocketHandle ServerSocket::accept_client()
{
    int client_fd = accept(socket_.get(), nullptr, nullptr);
    if (client_fd < 0)
    {
        throw std::runtime_error("accept 실패");
    }

    std::cout << "새 클라이언트 연결: fd=" << client_fd << "\n";
    return SocketHandle(client_fd);
}