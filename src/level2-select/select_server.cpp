#include "Socket.h"
#include <iostream>
#include <vector>
#include <cstring>

int main()
{
    try
    {
        std::cout << "=== SELECT 기반 ECHO Server 시작 ===\n";

        // ServerSocket으로 서버 초기화 (생성/바인딩/리슨 자동)
        ServerSocket server(8080);

        // 클라이언트 목록
        std::vector<SocketHandle> clients;

        // 메인 루프
        while (true)
        {
            fd_set read_fds;
            FD_ZERO(&read_fds);

            // 서버 소켓 감시
            FD_SET(server.get_fd(), &read_fds);
            int max_fd = server.get_fd();

            // 클라이언트 소켓들 감시
            for (const auto &client : clients)
            {
                FD_SET(client.get(), &read_fds);
                if (client.get() > max_fd)
                {
                    max_fd = client.get();
                }
            }

            std::cout << "select() 대기 중... (현재 클라이언트: " << clients.size() << "명)\n";

            // select 호출
            int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
            if (activity < 0)
            {
                throw std::runtime_error("select 실패");
            }

            // 새 접속 처리
            if (FD_ISSET(server.get_fd(), &read_fds))
            {
                clients.push_back(server.accept_client());
            }

            // 기존 클라이언트 메시지 처리
            for (auto it = clients.begin(); it != clients.end();)
            {
                if (FD_ISSET(it->get(), &read_fds))
                {
                    std::vector<char> buffer(1024);
                    std::fill(buffer.begin(), buffer.end(), 0);

                    ssize_t bytes = read(it->get(), buffer.data(), buffer.size() - 1);

                    if (bytes <= 0)
                    {
                        std::cout << "클라이언트 " << it->get() << " 연결 종료\n";
                        it = clients.erase(it);
                    }
                    else
                    {
                        buffer[bytes] = '\0';
                        std::cout << "[fd=" << it->get() << "] 받음: " << buffer.data();
                        write(it->get(), buffer.data(), bytes);
                        ++it;
                    }
                }
                else
                {
                    ++it;
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