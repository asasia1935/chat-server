#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>

class SocketHandle
{
private:
    int fd_;

public:
    // 생성자 : 소켓 파일 디스크립터를 받아서 관리 시작
    explicit SocketHandle(int fd) : fd_(fd)
    {
        // 생성 실패 예외처리
        if (fd_ < 0)
        {
            throw std::runtime_error("잘못된 소켓 파일 디스크립터");
        }
    }

    // 소멸자 : 객체가 사라질때 자동으로 소켓 닫도록 함 (RAII)
    ~SocketHandle()
    {
        if (fd_ >= 0)
        {
            close(fd_);
            std::cout << "소켓 " << fd_ << " 닫힘\n";
        }
    }

    // 복사 금지 (소켓은 하나만 관리해야 함)
    SocketHandle(const SocketHandle &) = delete;
    SocketHandle &operator=(const SocketHandle &) = delete;

    // Move semantics (소유권 이전)
    SocketHandle(SocketHandle &&other) noexcept : fd_(other.fd_)
    {
        other.fd_ = -1; // 원본은 무효화
    }

    SocketHandle &operator=(SocketHandle &&other) noexcept
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

    // getter: 실제 fd 값 반환
    int get() const { return fd_; }

    // 유효성 체크
    bool is_valid() const { return fd_ >= 0; }
};

int main()
{
    try
    {
        std::cout << "=== ECHO Server 시작 ===\n";

        /////////////////////
        // 1단계 : 소켓 설정 //
        /////////////////////
        // SocketHandle로 소켓의 반환값을 감싸면 자동으로 close 해줌
        SocketHandle server_fd(socket(AF_INET, SOCK_STREAM, 0));

        //////////////////////////
        // 2단계 : 서버 주소 설정 //
        /////////////////////////
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(8080);

        /////////////////////////////
        // 3단계 : 소켓에 주소 바인딩 //
        /////////////////////////////
        if (bind(server_fd.get(), (sockaddr *)&address, sizeof(address)) < 0)
        {
            throw std::runtime_error("바인딩 실패");
        }

        //////////////////////////////////
        // 4단계: 클라이언트 연결 대기 모드 //
        //////////////////////////////////
        if (listen(server_fd.get(), 3) < 0)
        {
            throw std::runtime_error("listen 실패");
        }
        std::cout << "서버 시작! 포트 8080에서 대기 중...\n";

        //////////////////////////////
        // 5단계: 클라이언트 연결 수락 //
        //////////////////////////////
        SocketHandle client_fd(accept(server_fd.get(), nullptr, nullptr));
        std::cout << "클라이언트 연결됨!\n";

        ////////////////////////////////////
        // 6단계: 에코 루프 (메시지 주고받기) //
        ////////////////////////////////////
        // C 배열 대신 std::vector 사용
        std::vector<char> buffer(1024);

        // 무한 반복 (클라이언트가 연결 끊을 때까지)
        while (true)
        {
            // 버퍼를 0으로 초기화 (이전 데이터 초기화)
            std::fill(buffer.begin(), buffer.end(), 0);

            ssize_t bytes = read(client_fd.get(), buffer.data(), buffer.size() - 1);

            if (bytes <= 0)
            {
                std::cout << "클라이언트 연결 종료\n";
                break;
            }

            // null-terminator 보장 -> 한글 출력 가능
            buffer[bytes] = '\0';

            // 받은 메시지 화면에 출력
            std::cout << "받음: " << buffer.data();

            write(client_fd.get(), buffer.data(), bytes);
        }

        // 중괄호 끝난 후 각 클래스의 소멸자 호출
    }
    catch (const std::exception &e)
    {
        std::cerr << "에러: " << e.what() << "\n";
        return 1;
    }

    std::cout << "서버 종료\n";
    return 0;
}