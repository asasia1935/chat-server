# 실시간 채팅 서버
C++ 멀티스레드 채팅 서버 프로젝트

## 개발 환경
- 언어: C++17
- 플랫폼: Linux (WSL2 Ubuntu 24.04)
- 빌드: g++ 13.3.0

## 현재 진행 상황

### v0.1 - Echo Server
- 1:1 클라이언트 연결
- 메시지 에코 기능
- TCP 소켓 기반 통신

## 빌드 & 실행
```bash
# 컴파일
g++ -o echo_server src/echo_server.cpp -std=c++17

# 실행
./echo_server

# 테스트 (다른 터미널)
telnet localhost 8080
```

## 다음 단계
- [ ] 다중 클라이언트 처리 (Thread Pool)
- [ ] 메시지 브로드캐스팅
- [ ] 닉네임 시스템
- [ ] 한글 처리

## 학습 목표
서버 개발자로의 커리어 전환을 위한 포트폴리오 프로젝트