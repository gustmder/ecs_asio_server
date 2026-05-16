# ECS Async Game Server

C++17 / Asio 기반 비동기 TCP 게임 서버.  
ECS(Entity-Component-System) 아키텍처와 맵별 전용 스레드로 게임 로직을 처리합니다.

---

## 기술 스택

| 분류 | 내용 |
|------|------|
| 언어 | C++17 |
| 네트워크 | Asio standalone (헤더 전용) |
| 로깅 | spdlog (헤더 전용) |
| 빌드 | CMake 3.20+ |

---

## 구조 요약

```
클라이언트
    │  TCP (프레임 v1.5)
    ▼
asio_server ──→ asio_channel (채널당)
                    │  frame_codec 파싱
                    ▼
              message_dispatcher
                    │  cmd → 핸들러
                    ▼
              game_server (net_handler)
                    │
          ┌─────────┴──────────┐
          ▼                    ▼
    ECS GameService      MapThread × N
    (Entity/Component    (맵당 전용 스레드)
     /System 관리)
```

### 네트워크 레이어

- 완전 비동기 (`async_read_some` / `asio::async_write`)
- **프레임 포맷 v1.5** — 16바이트 고정 헤더 + payload
  ```
  [len(4)][cmd(2)][flags(1)][ver(1)][seq(4)][crc32(4)][payload...]
  ```
- 채널별 타이머: idle timeout(30s), ping(10s), 통계 출력(5s)
- 백프레셔: write 큐 256KB 초과 시 연결 종료

### ECS 엔진

- `Entity` — uint32_t ID
- `Component` — 순수 데이터 (`Position`, `Velocity`, `Health`, `Player` 등)
- `System` — 순수 로직 (`MovementSystem`, `CombatSystem`, `AISystem`, `MapSystem`)
- `ComponentStore<T>` — 타입별 독립 저장소, 스레드 안전

### 스레딩 모델

```
main thread
  └── io_context (N IO 스레드)

MainThreadManager
  └── MapThread × 3  (Forest / Dungeon / City)
        └── packet_queue + priority task_queue
```

접속한 플레이어는 기본 맵(Forest)으로 배정되며,  
이동 패킷은 해당 맵 스레드로 라우팅됩니다.

---

## 빌드

```bash
# 의존성: third_party/asio, third_party/spdlog (git clone 또는 submodule)
cmake -S . -B build
cmake --build build -j4
```

빌드 결과물:

| 실행파일 | 설명 |
|----------|------|
| `build/game_server` | 게임 서버 |
| `build/simple_client` | 데모 클라이언트 |
| `build/test_*` | 각종 테스트 |

---

## 실행

```bash
# 터미널 1 — 서버 시작 (포트 12345)
./build/game_server

# 터미널 2 — 데모 클라이언트 (LOGIN → MOVE → CHAT → PING)
./build/simple_client
```

정상 실행 시 서버 콘솔:

```
[INFO] Server listening on 0.0.0.0:12345
[game_server] accept: channel=1
[GAME] Player login: TestPlayer (channel: 1)
[GAME] Player position updated (Entity: 1)
[GAME] Chat from channel 1: 안녕하세요!
```

---

## 메시지 ID 규약

| cmd | 방향 | 설명 |
|-----|------|------|
| 1 | C→S | PING |
| 2 | S→C | PONG |
| 1001 | C→S | 로그인 (payload: 플레이어 이름) |
| 1002 | C→S | 이동 (payload: x/y/z float × 3) |
| 1002 | S→C | 이동 브로드캐스트 (channel_id + x/y/z) |
| 1003 | C→S | 채팅 (payload: 메시지 문자열) |
| 1003 | S→C | 채팅 브로드캐스트 (channel_id + 메시지) |
| 1004 | C→S | 공격 (payload: target_id) |

---

## 현재 구현 상태

### 완료
- Asio 비동기 네트워크 레이어 (read/write 배칭, 백프레셔, 타이머)
- ECS 엔진 (Entity/Component/System 분리)
- 맵별 전용 스레드 및 패킷 라우팅
- 이동·채팅 브로드캐스트
- v1.5 프레임 포맷 (CRC32 포함)

### 미구현 (예정)
- DB 연동 (플레이어 데이터 영속화)
- 로그인 인증 / 세션 관리
- 공격 로직 (데미지 계산, 결과 브로드캐스트)
- 몬스터 / NPC 스폰
- 길드 시스템

> DB 레이어는 인메모리 ECS 구조가 안정화된 이후 추가 예정입니다.

---

## 문서

- [아키텍처 상세](docs/ARCHITECTURE.md)
- [개선 로드맵](docs/IMPROVEMENT_ROADMAP.md)
