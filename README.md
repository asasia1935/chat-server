# 실시간 채팅 서버
C++ 고성능 네트워크 서버 프로젝트 - Select부터 Epoll ET까지 단계별 구현

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
│   ├── level2-select/   # select() 기반 멀티클라이언트
│   │   └── select_server.cpp
│   ├── level3-poll/     # poll() 기반
│   │   └── poll_server.cpp
│   ├── level4-epoll-lt/ # epoll LT 모드
│   │   └── epoll_lt_server.cpp
│   └── level5-epoll-et/ # epoll ET 모드
│       └── epoll_et_server.cpp
└── build/               # 빌드 결과물
```

## 현재 진행 상황

### Level 1 - Basic Echo Server ✅
**단일 클라이언트 연결 기반 Echo 서버**

* TCP 소켓 기본 구조 학습
* 1:1 메시지 에코 기능
* 소켓 생성 → 바인딩 → 리슨 → Accept 흐름 이해

**빌드 & 실행:**
```bash
cd build
cmake ..
make level1_echo_server
./level1_echo_server
```

---

### Level 2 - Multi-Client Server (select) ✅
**select() 기반 I/O 멀티플렉싱**

**주요 기술:**
- **I/O 멀티플렉싱**: `select()` 시스템 콜
- **동시 다중 클라이언트** 처리 (싱글 스레드)
- **RAII 패턴**: `SocketHandle` 클래스로 자동 리소스 관리
- **소켓 추상화**: `ServerSocket` 클래스 분리

**핵심 구현:**
- `fd_set` 기반 소켓 관리
- `max_fd` 추적
- 클라이언트 동적 추가/제거
- 각 클라이언트별 독립적 에코 처리

**한계점:**
- `FD_SETSIZE` 제한 (보통 1024)
- `max_fd` 관리 필요
- 매번 `FD_SET` 재설정 필요

**빌드 & 실행:**
```bash
make level2_select_server
./level2_select_server
```

---

### Level 3 - Poll-based Server ✅
**poll() 기반 I/O 멀티플렉싱**

**Select → Poll 개선사항:**
- `fd_set` → `pollfd` 구조체 배열
- `FD_SETSIZE` 제한 제거
- `max_fd` 관리 불필요
- 더 직관적인 API (`events`, `revents`)

**핵심 개념:**
- `pollfd` 구조체: `{fd, events, revents}`
- `POLLIN`, `POLLOUT` 이벤트 플래그
- 배열 기반 관리로 확장성 개선

**성능:**
- Select와 동일한 O(n) 복잡도
- 대규모 연결에서도 fd 제한 없음

**빌드 & 실행:**
```bash
make level3_poll_server
./level3_poll_server
```

---

### Level 4 - Epoll LT (Level-Triggered) ✅
**Linux 전용 고성능 I/O 멀티플렉싱**

**Poll → Epoll LT 개선사항:**
- **O(1) 복잡도**: 이벤트 발생한 fd만 반환
- 커널이 fd 목록 관리 (매번 전달 불필요)
- 대규모 동시 연결 처리 최적화

**핵심 API:**
- `epoll_create1()`: epoll 인스턴스 생성
- `epoll_ctl()`: fd 추가/수정/삭제
- `epoll_wait()`: 이벤트 대기

**Level-Triggered 특징:**
- 데이터가 남아있으면 계속 알림
- Select/Poll과 동일한 동작 방식
- 안전하고 직관적

**빌드 & 실행:**
```bash
make level4_epoll_lt_server
./level4_epoll_lt_server
```

---

### Level 5 - Epoll ET (Edge-Triggered) ✅
**최고 성능 이벤트 기반 서버**

**LT → ET 개선사항:**
- **상태 변화 시에만 알림** (없음 → 있음)
- 같은 fd 반복 알림 방지
- 더 효율적인 이벤트 처리

**핵심 구현:**
- **논블로킹 소켓 필수**: `fcntl(O_NONBLOCK)`
- **while 루프**: 데이터 완전히 읽기
- **EAGAIN 처리**: "다 읽었음" 신호
- **EPOLLET 플래그**: ET 모드 활성화

**구현 패턴:**
```cpp
// 1. 논블로킹 설정
set_nonblocking(fd);

// 2. ET 모드 등록
ev.events = EPOLLIN | EPOLLET;

// 3. while 루프로 완전 읽기
while (true) {
    bytes = read(fd, buf, size);
    if (bytes < 0 && errno == EAGAIN) break;
}
```

**성능 특징:**
- 대규모 연결 시 LT 대비 3-5배 성능 향상
- CPU 사용률 감소
- 실전 서버 (Nginx, Redis 등) 사용 방식

**빌드 & 실행:**
```bash
make level5_epoll_et_server
./level5_epoll_et_server
```

**테스트:**
```bash
# netcat으로 접속 (한글 지원)
nc localhost 8080

# 여러 터미널에서 동시 접속 가능
```

---

## I/O 멀티플렉싱 성능 비교

| 방식 | 복잡도 | FD 제한 | 커널 지원 | 난이도 | 성능 |
|------|--------|---------|-----------|--------|------|
| Select | O(n) | 1024 | 모든 OS | 쉬움 | ⭐⭐ |
| Poll | O(n) | 무제한 | 모든 OS | 쉬움 | ⭐⭐⭐ |
| Epoll LT | O(1) | 무제한 | Linux | 보통 | ⭐⭐⭐⭐ |
| Epoll ET | O(1) | 무제한 | Linux | 어려움 | ⭐⭐⭐⭐⭐ |

---

## 다음 단계

### Level 6 - 멀티스레드 서버 (진행 예정)
* 스레드 풀 패턴
* 작업 큐 기반 요청 처리
* CPU 멀티코어 활용

## 학습 목표
* TCP/IP 소켓 프로그래밍 실전 경험
* **I/O 멀티플렉싱** 기법 비교 및 성능 이해
* **이벤트 기반 서버** 아키텍처 설계
* **고성능 네트워크 서버** 개발 능력 함양
* 서버 개발자로의 커리어 전환을 위한 포트폴리오 프로젝트

## 기술 스택
* **네트워크**: TCP Socket, I/O Multiplexing
* **동시성**: select/poll/epoll, Multi-threading
* **C++ 기법**: RAII, Smart Pointers, Move Semantics
* **빌드**: CMake, Makefile