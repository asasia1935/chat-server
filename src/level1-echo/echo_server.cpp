#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main()
{
    std::cout << "=== ECHO Server 시작 ===\n";

    /////////////////////
    // 1단계 : 소켓 설정 //
    /////////////////////
    // socket() : 소켓 생성
    // AF_INET : IPv4 인터넷 프로토콜 사용
    // SOCK_STREAM : TCP 방식
    // 0 : 기본 프로토콜 사용
    // 반환 값: 소켓 파일 디스크립터 (번호), 실패시 -1
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 소켓 생성 실패 확인
    if (server_fd == -1)
    {
        std::cerr << "소켓 생성 실패\n";
        return 1; // 프로그램 종료
    }

    //////////////////////////
    // 2단계 : 서버 주소 설정 //
    /////////////////////////
    sockaddr_in address;                  ///< sockaddr_in : 소켓 주소 구조체 (IP, Port 정보)
    address.sin_family = AF_INET;         ///< sin_family : 주소체계 (IPv4 사용)
    address.sin_addr.s_addr = INADDR_ANY; ///< sin_addr.s_addr : IP 주소, INADDR_ANY : 0.0.0.0
    address.sin_port = htons(8080);       ///< sin_port : 포트번호, htons : 네트워크 바이트 변환

    /////////////////////////////
    // 3단계 : 소켓에 주소 바인딩 //
    /////////////////////////////
    // bind() : 소켓에 IP 주소와 포트번호를 할당
    // (sockaddr*) : 형 변환 (sockaddr_in → sockaddr)
    // sizeof(address) : 구조체 크기 전달
    // 반환값: 성공 0, 실패 -1
    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "바인딩 실패\n";
        return 1;
    }

    //////////////////////////////////
    // 4단계: 클라이언트 연결 대기 모드 //
    //////////////////////////////////
    // listen() : 소켓을 수동 대기 모드로 전환
    // server_fd = 대기할 소켓
    // 3 = 대기 큐 크기 (최대 3개 연결 요청 대기 가능)
    listen(server_fd, 3);
    std::cout << "서버 시작! 포트 8080에서 대기 중...\n";

    //////////////////////////////
    // 5단계: 클라이언트 연결 수락 //
    //////////////////////////////
    // accept() : 클라이언트 연결 요청을 받아들임
    // 연결 올 때까지 여기서 멈춰있음 (blocking)
    // 반환값: 새로운 소켓 파일 디스크립터 (클라이언트와 통신용)
    // nullptr = 클라이언트 정보 안 받음
    int client_fd = accept(server_fd, nullptr, nullptr);
    std::cout << "클라이언트 연결됨!\n";

    ////////////////////////////////////
    // 6단계: 에코 루프 (메시지 주고받기) //
    ////////////////////////////////////
    // 버퍼 = 데이터를 임시 저장할 공간 (1024바이트)
    char buffer[1024];

    // 무한 반복 (클라이언트가 연결 끊을 때까지)
    while (true)
    {
        // memset() = 버퍼를 0으로 초기화 (이전 데이터 지우기)
        memset(buffer, 0, sizeof(buffer));

        // read() : 클라이언트로부터 데이터 읽기
        // client_fd = 읽을 소켓
        // buffer = 데이터 저장할 곳
        // sizeof(buffer) - 1 = 최대 읽을 크기 (마지막은 널 문자용)
        // 반환값: 읽은 바이트 수
        int bytes = read(client_fd, buffer, sizeof(buffer) - 1);

        // 연결 종료 확인
        // bytes <= 0 이면 클라이언트가 연결 끊음
        if (bytes <= 0)
        {
            std::cout << "클라이언트 연결 종료\n";
            break; // 반복문 탈출
        }

        // 받은 메시지 화면에 출력
        std::cout << "받음: " << buffer;

        // write() : 클라이언트에게 데이터 전송 (에코)
        // client_fd : 보낼 소켓
        // buffer : 보낼 데이터
        // bytes : 보낼 데이터 크기
        write(client_fd, buffer, bytes);
    }

    // ============================================
    // 7단계: 정리 (소켓 닫기)
    // ============================================
    // close() : 소켓 연결 종료 및 자원 해제
    close(client_fd); // 클라이언트 소켓 닫기
    close(server_fd); // 서버 소켓 닫기

    return 0;
}