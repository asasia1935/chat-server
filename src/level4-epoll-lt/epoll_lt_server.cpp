#include "Socket.h"
#include <iostream>
#include <vector>
#include <sys/epoll.h>
#include <algorithm>

// 클라이언트 이벤트?
constexpr int MAX_EVENTS = 10;

int main()
{
    try
    {
        std::cout << "=== EPOLL 기반 ECHO Server 시작 ===\n";

        // ServerSocket으로 서버 초기화 (생성/바인딩/리슨 자동)
        ServerSocket server(8080);

        // Epoll 인스턴스 생성
        int epoll_fd = epoll_create1(0);
        if (epoll_fd < 0)
        {
            throw std::runtime_error("epoll_create1 실패");
        }

        // 서버 소켓을 epoll에 등록
        epoll_event serverEv;
        serverEv.events = EPOLLIN;
        serverEv.data.fd = server.get_fd();

        // 0 미만이면 실패
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server.get_fd(), &serverEv) < 0)
        {
            throw std::runtime_error("epoll_ctl 실패");
        }

        // 클라이언트 소켓 목록 배열 -> pollfd와 동기화
        std::vector<SocketHandle> clients;

        // 이벤트 배열 (epoll_wait 결과를 받을 배열)
        epoll_event events[MAX_EVENTS];

        // 메인 루프
        while (true)
        {
            std::cout << "epoll_wait() 대기 중... (현재 클라이언트: "
                      << clients.size() << "명)\n";

            // 이벤트 대기 (한번만 호출)
            int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

            if (nfds < 0)
            {
                throw std::runtime_error("epoll_wait 실패");
            }

            // 발생한 이벤트만 순회
            for (size_t i = 0; i < nfds; i++)
            {
                int fd = events[i].data.fd; // 서버, 클라 가리지 않는 fd

                if (events[i].events & EPOLLIN)
                {
                    // 직접 fd값과 서버 및 클라의 fd 값을 비교
                    // 서버 소켓: 새 연결
                    if (fd == server.get_fd())
                    {
                        SocketHandle new_client = server.accept_client();

                        // 새 클라이언트를 epoll에 등록
                        epoll_event client_ev{};
                        client_ev.events = EPOLLIN;
                        client_ev.data.fd = new_client.get();

                        // epoll_ctl로 새 클라이언트 추가
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client.get(), &client_ev) < 0)
                        {
                            throw std::runtime_error("클라이언트 epoll_ctl 실패");
                        }

                        clients.push_back(std::move(new_client));
                    }
                    // 클라이언트 소켓: 데이터 수신
                    else
                    {
                        std::vector<char> buffer(1024);
                        std::fill(buffer.begin(), buffer.end(), 0);

                        ssize_t bytes = read(fd, buffer.data(), buffer.size() - 1);

                        if (bytes <= 0)
                        {
                            // 연결 종료
                            std::cout << "클라이언트 " << fd << " 연결 종료\n";

                            // epoll에서 제거
                            if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) < 0)
                            {
                                std::cerr << "epoll_ctl DEL 실패\n";
                            }

                            // clients 벡터에서 찾아서 제거
                            auto it = std::find_if(clients.begin(), clients.end(),
                                                   [fd](const SocketHandle &s)
                                                   { return s.get() == fd; });

                            if (it != clients.end())
                            {
                                clients.erase(it); // RAII로 자동 close
                            }
                        }
                        else
                        {
                            // Echo back
                            buffer[bytes] = '\0';
                            std::cout << "[fd=" << fd << "] 받음: " << buffer.data();
                            write(fd, buffer.data(), bytes);
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "에러: " << e.what() << "\n";
        return 1;
    }

    return 0;
}