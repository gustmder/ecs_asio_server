# 📦 Protocol 파일 구조 가이드

## 🎯 분리된 Protocol 구조

### **📁 파일 구조**
```
shared/
├── protocol_base.hpp          # 기본 유틸리티 + 로그인 메시지
├── protocol_player.hpp         # 플레이어 관련 메시지
├── protocol_chat.hpp          # 채팅 관련 메시지
├── protocol_guild.hpp         # 길드 관련 메시지
├── protocol_map.hpp           # 맵 관련 메시지
├── protocol_combat.hpp        # 전투 관련 메시지
└── protocol_all.hpp           # 모든 프로토콜 통합 헤더
```

### **🔧 사용 방법**

#### **1. 특정 기능만 사용하는 경우**
```cpp
// 플레이어 이동만 필요한 경우
#include "shared/protocol_base.hpp"
#include "shared/protocol_player.hpp"

// 채팅만 필요한 경우
#include "shared/protocol_base.hpp"
#include "shared/protocol_chat.hpp"
```

#### **2. 모든 기능을 사용하는 경우**
```cpp
// 모든 프로토콜이 필요한 경우
#include "shared/protocol_all.hpp"
```

## 📋 각 파일별 내용

### **🔧 protocol_base.hpp**
- **공통 유틸리티**: Little-Endian 인코딩/디코딩
- **메시지 ID**: 모든 메시지 ID 정의
- **기본 메시지**: echo_req, echo_res
- **로그인 메시지**: login_req, login_res

### **👤 protocol_player.hpp**
- **플레이어 이동**: player_move_req, player_move_res
- **플레이어 정보**: player_info_req, player_info_res
- **플레이어 상태**: player_status_update

### **💬 protocol_chat.hpp**
- **채팅**: chat_req, chat_res, chat_broadcast
- **시스템 메시지**: chat_system_message
- **채팅 필터**: chat_filter_req, chat_filter_res

### **🏰 protocol_guild.hpp**
- **길드 생성**: guild_create_req, guild_create_res
- **길드 가입**: guild_join_req, guild_join_res
- **길드 탈퇴**: guild_leave_req, guild_leave_res
- **길드 정보**: guild_info_req, guild_info_res
- **길드 멤버**: guild_member_list_req, guild_member_list_res

### **🗺️ protocol_map.hpp**
- **맵 진입**: map_enter_req, map_enter_res
- **맵 퇴장**: map_leave_req, map_leave_res
- **맵 객체**: map_objects_update, map_object_info
- **맵 정보**: map_info_req, map_info_res

### **⚔️ protocol_combat.hpp**
- **공격**: attack_req, attack_res, damage_notify
- **스킬**: skill_use_req, skill_use_res
- **상태 효과**: status_effect_notify
- **전투 결과**: combat_result_notify

## 🚀 장점

### **1. 가독성 향상**
- **파일별 분리**: 각 기능별로 명확한 분리
- **코드 길이**: 파일당 200-300줄로 관리 가능
- **유지보수**: 특정 기능 수정 시 해당 파일만 수정

### **2. 성능 최적화**
- **선택적 포함**: 필요한 메시지만 포함
- **컴파일 시간**: 불필요한 코드 제거
- **메모리 사용량**: 사용하지 않는 메시지 제외

### **3. 개발 효율성**
- **병렬 개발**: 팀원별로 다른 파일 담당 가능
- **버전 관리**: 파일별로 변경 이력 추적
- **테스트**: 기능별로 독립적인 테스트 가능

## 📝 사용 예시

### **서버에서 사용**
```cpp
// game_server.cpp
#include "shared/protocol_all.hpp"

void handle_login(const char* data, size_t size) {
    lemondory::shared::login_req req;
    if (lemondory::shared::login_req::parse(data, data + size, req)) {
        // 로그인 처리
    }
}
```

### **클라이언트에서 사용**
```cpp
// client.cpp
#include "shared/protocol_base.hpp"
#include "shared/protocol_player.hpp"

void send_move(float x, float y, float z) {
    lemondory::shared::player_move_req req;
    req.x = x; req.y = y; req.z = z;
    req.rotation = 0.0f;
    req.timestamp = get_current_time();
    
    auto data = req.serialize();
    send_to_server(data);
}
```

## 🔄 기존 파일 정리

### **삭제할 파일**
- `game/protocol/game_messages.hpp` (통합됨)
- `game/protocol/codec_le.hpp` (protocol_base.hpp로 이동)

### **유지할 파일**
- `shared/protocol_*.hpp` (새로운 구조)
- `shared/protocol_all.hpp` (통합 헤더)

## 🎯 다음 단계

1. **기존 코드 수정**: `#include` 경로 변경
2. **테스트**: 각 메시지 타입 동작 확인
3. **문서화**: 새로운 구조에 대한 팀 가이드 작성
4. **클라이언트 연동**: 공통 `shared/` 폴더 사용

---

**📝 이 구조로 프로토콜을 체계적으로 관리할 수 있습니다!** 🚀

