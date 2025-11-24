# 실시간 채팅 서버
C++ 멀티스레드 채팅 서버 프로젝트

## 개발 환경
- 언어: C++17
- 플랫폼: Linux (WSL2 Ubuntu 24.04)
- 빌드 시스템: CMake 3.10+
- 컴파일러: g++ 13.3.0

## 프로젝트 구조
```
chat-server/
├── CMakeLists.txt
├── README.md
├── include/              # 공통 헤더
│   └── Socket.h         # 소켓 추상화 클래스
├── src/
│   ├── common/          # 공통 구현
│   │   └── Socket.cpp
│   ├── level1-echo/     # 단일 클라이언트 Echo
│   │   └── echo_server.cpp
│   └── level2-select/   # select() 기반 멀티클라이언트
│       └── select_server.cpp
└── build/               # 빌드 결과물
```

## 현재 진행 상황

### Level 1 - Basic Echo Server ✅
* 단일 클라이언트 연결
* 1:1 메시지 에코 기능
* TCP 소켓 기본 구조 학습

**빌드 & 실행:**
```bash
cd build
cmake ..
make
./level1_echo_server
```

### Level 2 - Multi-Client Server (select) ✅
* **I/O 멀티플렉싱**: `select()` 기반
* **동시 다중 클라이언트** 처리
* **RAII 패턴**: `SocketHandle` 클래스로 자동 리소스 관리
* **소켓 추상화**: `ServerSocket` 클래스 분리

**주요 기능:**
- 클라이언트 동적 추가/제거
- 각 클라이언트별 독립적 에코 처리
- 소켓 fd 자동 정리 (RAII)

**빌드 & 실행:**
```bash
cd build
cmake ..
make
./level2_select_server
```

**테스트:**
```bash
# 여러 터미널에서 동시 접속 가능
telnet localhost 8080
```

## 다음 단계

### Level 3 - poll() 전환 (진행 예정)
* `select()` → `poll()`로 I/O 멀티플렉싱 변경
* fd_set 대신 pollfd 배열 사용
* FD_SETSIZE 제한 극복

## 학습 목표
* TCP/IP 소켓 프로그래밍 실전 경험
* I/O 멀티플렉싱 기법 비교 (select/poll/epoll)
* 멀티스레드 서버 아키텍처 설계
* 서버 개발자로의 커리어 전환을 위한 포트폴리오 프로젝트

## 기술 스택
* **네트워크**: TCP Socket, I/O Multiplexing
* **동시성**: select/poll/epoll, Multi-threading
* **C++ 기법**: RAII, Smart Pointers, Move Semantics
* **빌드**: CMake, Makefile