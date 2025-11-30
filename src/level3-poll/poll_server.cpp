#include "Socket.h"
#include <iostream>
#include <vector>
#include <poll.h>

int main()
{
    try
    {
        std::cout << "=== POLL 기반 ECHO Server 시작 ===\n";

        // ServerSocket으로 서버 초기화 (생성/바인딩/리슨 자동)
        ServerSocket server(8080);

        // pollfd 배열 - [0]은 서버 소켓, [1~]부터 클라이언트
        std::vector<pollfd> poll_fds;

        // 서버 소켓을 poll_fds에 추가
        pollfd server_pfd{};
        server_pfd.fd = server.get_fd();
        server_pfd.events = POLLIN; // 이 fd에서 읽기 이벤트 감지
        server_pfd.revents = 0;
        poll_fds.push_back(server_pfd);

        // 클라이언트 소켓 목록 배열 -> pollfd와 동기화
        std::vector<SocketHandle> clients;

        // 메인 루프
        while (true)
        {
            std::cout << "poll() 대기 중... (현재 클라이언트: "
                      << clients.size() << "명)\n";

            // poll() 호출 (타임아웃 -1 : 무한 대기)
            // pollfd 배열의 포인터, 배열의 크기, 타임아웃 값을 받아서 이벤트 발생한 갯수 반환
            int activity = poll(poll_fds.data(), poll_fds.size(), -1);

            if (activity < 0)
            {
                throw std::runtime_error("poll 실패");
            }

            // 모든 fd 검사
            for (size_t i = 0; i < poll_fds.size(); i++)
            {
                // 해당 fd에 이벤트 발생 없을 시 스킵
                if (poll_fds[i].revents == 0)
                {
                    continue;
                }

                // POLLIN : 읽을 데이터 있음
                if (poll_fds[i].revents && POLLIN)
                {
                    // 서버 소켓(인덱스 0)일 경우는 새 연결
                    if (i == 0)
                    {
                        SocketHandle new_client = server.accept_client();

                        // 새 클라이언트를 poll_fds에 추가
                        pollfd client_pfd{};
                        client_pfd.fd = new_client.get();
                        client_pfd.events = POLLIN;
                        client_pfd.revents = 0;
                        poll_fds.push_back(client_pfd);

                        // clients 벡터에도 추가
                        clients.push_back(std::move(new_client));
                    }
                    // 클라이언트 소켓일 경우는 데이터 수신
                    else
                    {
                        int client_fd = poll_fds[i].fd;
                        size_t client_idx = i - 1; // clients 벡터의 인덱스

                        std::vector<char> buffer(1024);
                        std::fill(buffer.begin(), buffer.end(), 0);

                        ssize_t bytes = read(client_fd, buffer.data(), buffer.size() - 1);

                        if (bytes <= 0)
                        {
                            // 연결 종료 또는 에러
                            std::cout << "클라이언트 " << client_fd << " 연결 종료\n";

                            // poll_fds에서 제거
                            poll_fds.erase(poll_fds.begin() + i);

                            // clients에서 제거 -> 클라이언트 소켓 닫음
                            clients.erase(clients.begin() + client_idx);

                            --i; // 제거 했으니 인덱스 조정
                        }
                        else
                        {
                            // ECHO Back
                            buffer[bytes] = '\0';
                            std::cout << "[fd=" << client_fd << "] 받음: " << buffer.data();
                            write(client_fd, buffer.data(), bytes);
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