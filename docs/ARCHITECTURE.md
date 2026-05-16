# Architecture Overview

## 프로젝트 개요

Asio 기반 비동기 TCP 서버 위에 ECS(Entity-Component-System) 게임 엔진을 올린 C++17 멀티스레드 게임 서버.

| 항목 | 내용 |
|------|------|
| 언어 | C++17 |
| 네트워크 | Asio standalone (header-only) |
| 로깅 | spdlog (header-only) |
| 빌드 | CMake 3.20+, Debug 기본 |

---

## 디렉토리 구조

```
ecs_asio_server/
├── core/           lemondory_core 라이브러리
│   ├── packet_buffer   직렬화 버퍼
│   ├── thread_group    IO 스레드 풀
│   ├── spin_lock       경량 스핀락
│   └── config          설정 파싱
│
├── network/        lemondory_network 라이브러리
│   ├── asio_server     TCP acceptor, 채널 풀
│   ├── asio_channel    개별 TCP 연결 (read/write/timer)
│   ├── frame_codec     프레임 포맷 v1.5
│   ├── message_dispatcher  cmd → 핸들러 라우팅
│   └── net_handler     이벤트 인터페이스
│
├── game/           game_server 실행파일
│   ├── entity.*        Entity(uint32_t) + EntityManager
│   ├── component.*     ComponentStore<T> + ComponentManager
│   ├── system.*        System 베이스 + SystemManager
│   ├── game_service.*  ECS 퍼사드 싱글톤
│   ├── game_server.*   net_handler 구현
│   ├── components/     transform, health, map, game_object
│   ├── systems/        movement, combat, ai, map
│   └── threading/      MapThread (맵별 전용 스레드)
│
├── shared/         클라이언트-서버 공용 프로토콜 구조체
│   └── protocol_*.hpp  player, chat, combat, map, guild
│
└── tests/          테스트 실행파일
```

---

## 네트워크 레이어

### 프레임 포맷 (v1.5, 16바이트 고정 헤더)

```
[len(4 LE)][cmd(2)][flags(1)][ver(1)][seq(4)][crc32(4)][payload...]
```

- `len` : 헤더 이후 바이트 수 (= 12 + payload 크기)
- `cmd` : 메시지 ID
- `flags` : 압축/암호화 확장 포인트 (현재 미사용)
- `ver` : 프로토콜 버전 (현재 1)
- `seq` : 시퀀스 번호 (현재 미사용)
- `crc32` : payload CRC32 (IEEE)

### 비동기 처리 흐름

```
[수신]
async_read_some
  → frame_acc_(누적 버퍼)
  → parse_frames_()
  → on_frame_ 콜백
  → message_dispatcher::dispatch()
  → 핸들러 함수

[송신]
send()
  → enqueue_send_() [spin_lock]
  → make_batch_()   [최대 64KB / 64 메시지]
  → asio::async_write
  → 완료 콜백 → 다음 배치
```

### 채널 타이머

| 타이머 | 주기 | 동작 |
|--------|------|------|
| idle_timer | 30초 | 수신 없으면 연결 종료 |
| ping_timer | 10초 | PING 프레임 자동 전송 |
| stats_timer | 5초 | 큐/바이트 통계 출력 |

### 백프레셔 정책

write 큐가 256KB 초과 시 해당 채널을 `force_closed`로 종료.

---

## ECS 구조

### 계층

```
GameService (퍼사드 싱글톤)
  ├── EntityManager   Entity(uint32_t) 발급 / 관리
  ├── ComponentManager  ComponentStore<T> per type
  └── SystemManager   priority 기반 System 업데이트
```

### Component 저장 구조

```
ComponentManager
  └── stores_: unordered_map<type_index, unique_ptr<IComponentStore>>
        └── ComponentStore<T>: unordered_map<Entity, unique_ptr<T>>
```

### 구현된 컴포넌트

| 컴포넌트 | 필드 |
|----------|------|
| Position | x, y, z |
| Velocity | x, y, z |
| Rotation | x, y, z, w (쿼터니언) |
| Scale | x, y, z |
| Health | hp, max_hp |
| Player | name, level, channel_id |
| MapComponent | 맵 메타데이터 |

### 구현된 시스템

| 시스템 | 역할 |
|--------|------|
| MovementSystem | Velocity → Position 갱신, 경계 체크 |
| CombatSystem | 공격/피해/치유/사망 처리 |
| AISystem | AI 행동 갱신 |
| MapSystem | 맵 로드/관리 |

---

## 스레딩 모델

```
main thread
  └── io_context (thread_group, N개 IO 스레드)
        └── asio_channel (채널당 1개, strand 없음)

MainThreadManager
  └── MapThread × N (맵당 1개 전용 스레드)
        ├── packet_queue  (condition_variable 대기)
        └── task_queue    (priority_queue)
```

접속 시 플레이어는 기본 맵(map_id=1)으로 배정.  
이동 패킷은 `game_server`에서 해당 맵 스레드로 라우팅됨.

---

## 메시지 ID 규약

| 범위 | 용도 |
|------|------|
| 1 | PING |
| 2 | PONG |
| 100 | Echo (테스트) |
| 1001 | LOGIN |
| 1002 | MOVE |
| 1003 | CHAT |
| 1004 | ATTACK |

---

## 빌드

```bash
cmake -S . -B build
cmake --build build --target game_server
```
