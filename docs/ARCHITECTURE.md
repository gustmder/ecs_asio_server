# Architecture Overview

## 프로젝트 개요

Asio 기반 비동기 TCP 서버 위에 ECS(Entity-Component-System) 게임 엔진을 올린 C++17 멀티스레드 게임 서버.

| 항목 | 내용 |
|------|------|
| 언어 | C++17 |
| 네트워크 | Asio 1.38.0 standalone (header-only) |
| 로깅 | spdlog 1.17.0 (header-only) |
| 직렬화 | nlohmann/json 3.12.0 (설정 파일) |
| 빌드 | CMake 3.20+, Debug 기본 |

---

## 서버 토폴로지

### Phase 1 — 현재 구현

```
[Bot / Client]
      │  TCP (game protocol, session_token = "")
      ▼
[Game Server :12345]
      │
      ▼
[In-Memory ECS State]
```

- 인증 없음. `login_req.session_token`은 빈 문자열, 서버에서 무시.
- 단일 서버 프로세스, 인메모리 상태만 존재.

### Phase 2 — DB 연동 (구현 완료)

```
[Bot / Client]
      │  TCP (session_token = "")
      ▼
[Game Server :12345]
      │  per-player strand (asio::thread_pool 4 threads)
      ▼
[Game DB (MySQL)]
```

- 플레이어 데이터 영속화: 로그인 시 `find_or_create`, 로그아웃 시 위치·스탯 저장.
- DB 작업은 별도 thread_pool에서 비동기 처리 — io_context 블로킹 없음.
- `session_token`은 여전히 무시 (auth 서버 없음).

### Phase 3 — Auth / Front 서버 분리 (예정)

```
[Bot / Client]
      │
      │ 1) HTTP POST /auth/login → session_token (JWT)
      ▼
[Auth Server]  ←→ [Account DB]
      │
      │ 2) TCP connect + LOGIN {session_token}
      ▼
[Front Server]  ← token 검증(Auth 서버 호출), 게임 서버 라우팅
      │  내부 TCP
      ▼
[Game Server]  ←→ [Game DB]
```

### Phase 간 변경 범위 (최소화 설계)

클라이언트/봇은 두 함수만 교체하면 됨:

```cpp
// Phase 1 (현재)
std::string authenticate() { return ""; }
std::string game_host()    { return "localhost"; }
int         game_port()    { return 12345; }

// Phase 3 — 이 함수들만 바뀜
std::string authenticate() { return http_post("/auth/login", creds); }
std::string game_host()    { return "front-server"; }
int         game_port()    { return 10000; }
```

서버(handle_login)는 token 검증 로직 한 블록만 추가:

```cpp
// Phase 1: token 무시
// Phase 3: if (!auth_client.verify(token)) { reject(); return; }
```

---

## 디렉토리 구조

```
ecs_asio_server/
├── common/
│   ├── core/       packet_buffer (직렬화 버퍼)
│   ├── network/    frame_codec, net_handler, socket_channel_base,
│   │               message_dispatcher, message_ids
│   └── protocol/   protocol_*.hpp (클라이언트/서버 공유 패킷 구조체)
│
├── server/
│   ├── core/       server_config (JSON 설정), thread_group
│   ├── network/    asio_server, asio_channel
│   ├── db/         DB 인프라 (DbManager, DbConnectionPool, FailureSink)
│   │   └── dao/    도메인별 DAO (PlayerDao, …)
│   └── game/       게임 서버 루트 (game_server)
│       ├── ecs/          ECS 코어 (entity, component, system, game_service, memory_pool)
│       ├── objects/      게임 오브젝트 (player, …)
│       ├── components/   컴포넌트 정의 (transform, health, map, game_object)
│       ├── systems/      시스템 구현 (movement, combat, ai, map)
│       ├── threading/    스레딩 (MainThreadManager, MapThread)
│       └── handlers/     패킷 핸들러 (player_handlers, guild_handlers)
│
├── client/
│   ├── game_bot.hpp/cpp  GameBot 클래스 (auth/connect 분리)
│   └── main.cpp          봇 실행 진입점
│
├── config/
│   └── server_config.json
│
├── tests/
└── docs/
```

---

## 네트워크 레이어

### 프레임 포맷 (v1.5, 16바이트 고정 헤더)

```
[len(4 LE)][cmd(2)][flags(1)][ver(1)][seq(4)][crc32(4)][payload...]
```

| 필드 | 크기 | 설명 |
|------|------|------|
| len | 4 | 헤더 이후 바이트 수 (= 12 + payload) |
| cmd | 2 | 메시지 ID |
| flags | 1 | 압축/암호화 확장 포인트 (현재 미사용) |
| ver | 1 | 프로토콜 버전 (현재 1) |
| seq | 4 | 시퀀스 번호 (현재 미사용) |
| crc32 | 4 | payload CRC32 (IEEE) |

### 비동기 처리 흐름

```
[수신]
async_read_some → frame_acc_(누적 버퍼)
  → try_parse_one (offset 방식)
  → on_frame_ 콜백
  → message_dispatcher::dispatch()
  → 핸들러 함수

[송신]
send() → enqueue_send_() [spin_lock]
  → make_batch_() [최대 64KB / 64 메시지]
  → asio::async_write → 완료 콜백 → 다음 배치
```

---

## 메시지 ID 규약

| ID | 방향 | 이름 | 설명 |
|----|------|------|------|
| 1 | S↔C | PING | 헬스체크 |
| 2 | S↔C | PONG | 헬스체크 응답 |
| 100 | S↔C | ECHO | 테스트용 |
| 1001 | C→S | LOGIN | `login_req` (player_name, session_token) |
| 1002 | C→S | MOVE | `player_move_req` |
| 1003 | C→S | CHAT | 채팅 메시지 |
| 1004 | C→S | ATTACK | `attack_req` / S→C `damage_notify` |
| 1005 | C→S | MAP_ENTER | `map_enter_req` |
| 2001 | C→S | GUILD_CREATE | `guild_create_req` |
| 2002 | C→S | GUILD_JOIN | `guild_join_req` |
| 2003 | C→S | GUILD_LEAVE | `guild_leave_req` |

> **login_req 구조:**
> ```
> [player_name (lp_string)]
> [session_token (lp_string)]  ← Phase 1: "" / Phase 3: JWT
> ```

---

## ECS 구조

```
GameService (퍼사드 싱글톤)
  ├── EntityManager      Entity(uint32_t) 발급 / 관리
  ├── ComponentManager   ComponentStore<T> per type (snapshot 기반 이터레이션)
  └── SystemManager      priority 기반 System 업데이트
```

### 구현된 컴포넌트

| 컴포넌트 | 주요 필드 |
|----------|-----------|
| Position | x, y, z |
| Velocity | x, y, z |
| Health | hp, max_hp |
| Player | name, level, channel_id, map_id |
| Monster | type, level, target_entity |
| GameObject | type(enum), name, object_id |
| MapComponent | map_id, name, width, height, max_players |

### 구현된 시스템

| 시스템 | 역할 |
|--------|------|
| MovementSystem | Velocity → Position 갱신, 경계 체크 |
| CombatSystem | attack/take_damage/heal/get_hp |
| AISystem | AI 행동 (스텁) |
| MapSystem | 맵 로드/관리 (스텁) |

---

## 스레딩 모델

```
main thread
  └── asio io_context (단일 스레드)
        └── asio_channel per connection (strand 없음)
              └── on_message → handle_login / handle_move / ...

DB thread pool (asio::thread_pool, 4 threads)
  └── per-player strand (channel_id → db_strands_[channel_id])
        └── find_or_create / save_position / save_stats
              └── 완료 후 asio::post(io_exec_) → ECS 업데이트·응답 전송

MainThreadManager
  └── MapThread × N  (config.maps 배열 기준, 맵당 1개 전용 스레드)
        ├── packet_queue   (condition_variable 대기)
        ├── task_queue     (priority_queue)
        └── spawn_timer    (30초마다 몬스터 스폰)
```

### 스레드 간 규칙

| 스레드 | ECS 쓰기 | ECS 읽기 | DB 호출 |
|--------|----------|----------|---------|
| io_context | ✅ (배타적) | ✅ | ❌ (strand로 위임) |
| DB strand | ❌ | ❌ | ✅ |
| MapThread | ❌ (권고) | ✅ | ❌ |

- 접속 시 플레이어는 기본 맵(map_id=1)으로 배정.
- 이동/전투 패킷은 해당 맵 스레드로 라우팅, MAP_ENTER로 맵 이동 가능.
- 같은 플레이어의 DB 작업(login → logout)은 동일 strand로 자동 직렬화됨.

---

## 설정 파일 (config/server_config.json)

```json
{
  "server":    { "host", "port", "backlog", "max_connections", "tcp_no_delay", "tick_rate" },
  "logging":   { "level", "file", "file_path" },
  "threading": { "io_threads", "map_threads_enabled" },
  "game":      { "use_ecs", "memory_pool_size" },
  "maps":      [ { "id", "name", "width", "height", "max_players" }, ... ],
  "databases": {
    "game":   { "host", "port", "user", "password", "name", "pool_size", "enabled" },
    "guild":  { ... },
    "common": { ... }
  }
}
```

파일이 없거나 파싱 실패 시 코드 기본값으로 폴백.  
`databases.[role].enabled = false` 이면 해당 role의 pool은 생성되지 않음.

---

## DB 레이어

### 구조

```
DbManager
  ├── DbConnectionPool[Game]   (RAII acquire/release, acquire timeout 5s)
  ├── DbConnectionPool[Guild]
  └── DbConnectionPool[Common]

PlayerDao (Game pool 사용)
  ├── find_or_create()  INSERT IGNORE + SELECT (단일 커넥션, 레이스 방지)
  ├── save_position()   3회 재시도, retryable error만 재시도
  └── save_stats()      위와 동일

FailureSink
  └── logs/failed_saves.jsonl  (JSON Lines, 최종 실패 시 스냅샷)
```

### 재시도 정책

| MySQL 에러 코드 | 의미 | 재시도 |
|-----------------|------|--------|
| 2006 | 서버 연결 끊김 | ✅ |
| 2013 | 쿼리 중 연결 끊김 | ✅ |
| 1213 | 데드락 | ✅ |
| 1205 | 락 타임아웃 | ✅ |
| 그 외 | 논리 오류 등 | ❌ |

최대 3회 재시도 후 실패 시 `FailureSink`에 JSON Lines로 스냅샷 기록.

---

## 빌드

```bash
cmake -S . -B build
cmake --build build --target game_server   # 서버
cmake --build build --target game_bot      # 봇 클라이언트 (예정)
cmake --build build --parallel 4           # 전체
```
