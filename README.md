# ECS Asio Game Server

C++17 기반 멀티플레이어 게임 서버. ECS(Entity-Component-System) 아키텍처와 Asio 비동기 네트워크 레이어를 결합하고, 맵별 전용 스레드와 비동기 DB 레이어를 갖춘 구조입니다.

## 기술 스택

| 분류 | 내용 |
|---|---|
| 언어 | C++17 |
| 네트워크 | Asio 1.38.0 (standalone) |
| 로깅 | spdlog 1.17.0 |
| 설정 | nlohmann/json 3.12.0 |
| DB | MySQL / MariaDB (선택) |
| 빌드 | CMake 3.20+ |
| 컨테이너 | Docker + docker compose |

서드파티 라이브러리(Asio, spdlog, nlohmann/json)는 `third_party/`에 포함되어 있어 별도 설치가 필요 없습니다.

## 아키텍처

```
[Client]
   │ TCP — 16바이트 고정 헤더 + payload
   ▼
[Network Layer]  asio_server → asio_channel → frame_codec
   │
   ▼
[Game Server]  message_dispatcher → handlers
   │
   ├── [ECS Core]           entity / component / system / game_service
   ├── [Threading]          MainThreadManager → MapThread × N (맵별 전용 스레드)
   └── [DB Layer]           per-player strand → DAO → MySQL
                            FailureSink (DB 최종 실패 시 JSON Lines 스냅샷)
```

### 주요 구현 특징

- **ECS 패턴**: Entity/Component/System 분리로 게임 오브젝트 관리
- **맵별 스레드**: 각 맵이 독립 스레드에서 게임 루프 실행, 패킷은 맵 스레드로 라우팅
- **비동기 DB**: `asio::strand` 기반 per-player 직렬화로 io_context 블로킹 방지
- **Write-behind**: 위치/스탯은 30초 주기 일괄 저장, 재화/아이템은 즉시 저장
- **FailureSink**: DB 최종 실패 시 JSON Lines 스냅샷 기록 + stderr fallback
- **Graceful shutdown**: `db_pool_.join()`으로 진행 중 DB 작업 완료 후 종료

## 디렉토리 구조

```
ecs_asio_server/
├── common/
│   ├── core/           패킷 버퍼, 직렬화
│   ├── network/        프레임 코덱, 네트워크 추상화
│   └── protocol/       클라이언트-서버 공유 패킷 구조체
├── server/
│   ├── core/           서버 설정 (JSON 로딩)
│   ├── network/        Asio 서버/채널 구현
│   ├── db/             DB 인프라 (DbManager, ConnectionPool, FailureSink)
│   │   └── dao/        도메인 DAO (PlayerDao, ...)
│   └── game/
│       ├── ecs/        ECS 코어 (entity, component, system, game_service)
│       ├── objects/    게임 오브젝트 (player, ...)
│       ├── components/ 컴포넌트 정의 (transform, health, map, game_object)
│       ├── systems/    시스템 구현 (movement, combat, ai, map)
│       ├── threading/  스레딩 (MainThreadManager, MapThread)
│       └── handlers/   패킷 핸들러 (player, guild)
├── client/             테스트용 게임봇 클라이언트
├── config/             서버 설정 파일
├── docker/             DB 초기화 스크립트
├── docs/               아키텍처, 코딩 스타일, 개선 로드맵
└── tests/              유닛/통합/성능 테스트
```

## 빌드

### 요구사항

- CMake 3.20+
- C++17 컴파일러 (GCC 10+ / Clang 12+ / AppleClang 14+)
- MySQL 클라이언트 라이브러리 (선택 — 없으면 DB 레이어 비활성화)

### MySQL 없이 빌드

```bash
cmake -B build -S .
cmake --build build --parallel
```

### MySQL 포함 빌드

```bash
# macOS
brew install mysql-client

# Ubuntu/Debian
sudo apt-get install libmysqlclient-dev

cmake -B build -S .
cmake --build build --parallel
```

MySQL이 감지되면 `LEMONDORY_HAVE_MYSQL` 플래그가 자동 활성화됩니다.

빌드 결과물:

| 실행파일 | 설명 |
|---|---|
| `build/game_server` | 게임 서버 |
| `build/game_bot` | 테스트용 게임봇 클라이언트 |
| `build/test_ecs_system` | ECS 유닛 테스트 |
| `build/test_threading_system` | 스레딩 테스트 |
| `build/test_network_stability` | 네트워크 안정성 테스트 |

## 실행

### 1. 로컬 (DB 없이)

```bash
./build/game_server
```

### 2. Docker로 개발 환경 구성 (MySQL + Redis)

```bash
cp .env.example .env          # 필요 시 비밀번호 수정
docker compose -f docker-compose.dev.yml up -d
./build/game_server
```

### 3. 풀스택 Docker 배포

```bash
cp .env.example .env          # 비밀번호 반드시 변경
docker compose -f docker-compose.prod.yml up -d
```

## 패킷 프로토콜

16바이트 고정 헤더 + 가변 페이로드 구조입니다.

```
[len(4 LE)][cmd(2)][flags(1)][ver(1)][seq(4)][crc32(4)][payload...]
```

| cmd | 방향 | 기능 |
|---|---|---|
| 1 / 2 | C→S / S→C | PING / PONG |
| 1001 | C↔S | 로그인 |
| 1002 | C↔S | 이동 / 이동 브로드캐스트 |
| 1003 | C↔S | 채팅 |
| 1004 | C↔S | 공격 / 데미지 알림 |
| 1005 | C↔S | 맵 이동 |
| 2001 | C↔S | 길드 생성 |
| 2002 | C↔S | 길드 가입 |
| 2003 | C↔S | 길드 탈퇴 |

패킷 구조체는 `common/protocol/` 참고.

## 테스트

```bash
./build/test_ecs_system
./build/test_threading_system
./build/test_network_stability
```

## 문서

- [아키텍처 상세](docs/ARCHITECTURE.md)
- [코딩 스타일 가이드](docs/CODING_STYLE.md)
- [개선 로드맵](docs/IMPROVEMENT_ROADMAP.md)
