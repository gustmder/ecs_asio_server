# Improvement Roadmap

## 완료된 개선 사항

### 버그 수정
- **ComponentManager 메모리 누수** : `void*` raw 포인터 → `unique_ptr<IComponentStore>` 교체
- **listen() 이중 호출** : `game_server::init()`에서 중복 listen 제거
- **has_component 이중 락** : stores_mutex_ 보유 중 get_store() 재진입 제거

### 아키텍처 개선
- **channel_id → Entity O(1) 매핑** : 이동 처리 시 전체 순회(O n) → 해시맵 O(1) 조회
- **broadcast / broadcast_except** : 이동·채팅 결과를 다른 플레이어에게 전달하는 기반 구현
- **asio_server 채널 생명주기** : close 콜백으로 channels_ 자동 제거 (tick() 수동 호출 제거)
- **socket_channel_base 미사용 버퍼 제거** : write/send/read 3개 socket_buffer 제거 (채널당 ~128KB 낭비 제거)
- **frame_acc\_ O(n) erase → 오프셋 방식** : 읽기 위치만 전진, 절반 소비 시 한 번만 compact
- **CRC 검증 활성화** : 페이로드 CRC 불일치 프레임 드롭 처리

### 코드 품질
- **asio_channel.cpp 주석 정리** : 970줄 → 230줄 (이전 버전 코드 주석 전량 제거)
- **asio_server.cpp 주석 정리** : 140줄 주석 제거
- **CMake 빌드 구조 개선** : 테스트 7개 타겟이 game 소스 중복 컴파일 → `lemondory_game` 정적 라이브러리로 통합, CMakeLists.txt 하단 구버전 주석 제거
- **미사용 코드 제거** : `asio_channel.hpp` 죽은 멤버 6개, `socket_buffer` 클래스 전체, `frame_codec.hpp` 구버전 오버로드 + 주석 블록 제거 (448줄 → 136줄)

---

## 다음 단계: 아키텍처 개선

### 우선순위 높음

| 항목 | 현황 | 목표 |
|------|------|------|
| spdlog 활성화 | std::cout/cerr fallback | CMake 연결 정상화 |
| 설정 파일 로딩 | config/ 미사용 | 서버 시작 시 json/ini 로드 |

### 우선순위 중간

| 항목 | 설명 |
|------|------|
| 공간 인덱스 | get_entities_in_range() 현재 선언만 존재, Grid 또는 Quadtree 필요 |
| ~~ComponentStore 이터레이션 안전성~~ | begin/end 제거, components\_/mutex\_ private화, snapshot() 메서드로 안전한 이터레이션 보장 ✓ |

---

## 다음 단계: 기능 구현

### DB 레이어 (미구현)
- 플레이어 데이터 저장/로드 (MySQL 또는 Redis)
- 로그인 인증 (세션 토큰)
- 인벤토리/아이템 영속화

### 게임 로직 (스텁 상태)
- 공격 로직: 타겟 검증, 데미지 계산, 결과 브로드캐스트
- 몬스터/NPC 스폰: MapThread 내 핸들러 구현
- 맵 이동(존 이동): 채널을 다른 MapThread로 재배정

### 인프라
- SSL/TLS 지원
- graceful shutdown (pending write flush 후 종료)
- Rate limiting (패킷 빈도 제한)

---

## 알려진 제약 / 의도적 미구현

- **길드 시스템** : `protocol_guild.hpp` 정의 완료, 핸들러 미등록 (DB 이후 구현 예정)
- **인벤토리** : 프로토콜 미정의
- **전투 결과 검증** : 서버-권위 구조 미적용 (클라이언트 신뢰 방식)
