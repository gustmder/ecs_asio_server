# 📝 Lemondory Game Server - 코딩 스타일 가이드

## 🎯 C++ 코딩 스타일 규칙

### **1. 중괄호 스타일 (Brace Style)**
```cpp
// ✅ 권장: K&R 스타일 (현대 C++ 표준)
void function_name() {
    // 함수 본문
}

class MyClass {
public:
    void method() {
        // 메서드 본문
    }
    
private:
    int member_;
};

// ❌ 비권장: Allman 스타일 (Java 스타일)
void function_name() 
{
    // 함수 본문
}
```

### **2. 네임스페이스 스타일**
```cpp
// ✅ 권장
namespace lemondory::game {
    class GameService {
    public:
        void update() {
            // 구현
        }
    };
} // namespace lemondory::game

// ❌ 비권장
namespace lemondory::game 
{
    class GameService 
    {
    public:
        void update() 
        {
            // 구현
        }
    };
}
```

### **3. 클래스 정의 스타일**
```cpp
// ✅ 권장
class GameServer {
public:
    GameServer() = default;
    ~GameServer() = default;
    
    bool init(asio::io_context& io, const socket_option& opt,
              const std::string& ip, int port) {
        // 구현
    }
    
private:
    std::shared_ptr<asio_server> server_;
    bool use_ecs_system_ = true;
};

// ❌ 비권장
class GameServer 
{
public:
    GameServer() = default;
    ~GameServer() = default;
    
    bool init(asio::io_context& io, const socket_option& opt,
              const std::string& ip, int port) 
    {
        // 구현
    }
    
private:
    std::shared_ptr<asio_server> server_;
    bool use_ecs_system_ = true;
};
```

### **4. 제어문 스타일**
```cpp
// ✅ 권장
if (condition) {
    // 실행
} else {
    // 실행
}

for (int i = 0; i < count; ++i) {
    // 실행
}

while (condition) {
    // 실행
}

// ❌ 비권장
if (condition) 
{
    // 실행
} 
else 
{
    // 실행
}
```

### **5. 함수 선언 스타일**
```cpp
// ✅ 권장: 매개변수별 줄바꿈
bool init(asio::io_context& io, 
          const socket_option& opt,
          const std::string& ip, 
          int port) {
    // 구현
}

// ✅ 권장: 짧은 함수는 한 줄
void update() { /* 구현 */ }

// ❌ 비권장: 불필요한 줄바꿈
bool init(
    asio::io_context& io, 
    const socket_option& opt,
    const std::string& ip, 
    int port
) 
{
    // 구현
}
```

## 🏗️ 아키텍처별 스타일 가이드

### **네트워크 레이어**
```cpp
// ✅ 권장
class asio_server {
public:
    bool init(net_handler* handler, const socket_option& opt, 
              int max_channels, int thread_count) {
        // 구현
    }
    
    void on_accept(int channel_id, socket_channel_base* channel) {
        // 구현
    }
};
```

### **ECS 레이어**
```cpp
// ✅ 권장
template<typename T>
bool has_component(Entity entity) const {
    auto it = stores_.find(std::type_index(typeid(T)));
    if (it == stores_.end()) {
        return false;
    }
    // 구현
}

template<typename T>
T* get_component(Entity entity) {
    return get_store<T>().get(entity);
}
```

### **스레딩 레이어**
```cpp
// ✅ 권장
void MainThreadManager::add_task(std::function<void()> task, int priority) {
    std::lock_guard<std::mutex> lock(task_mutex_);
    task_queue_.emplace(std::move(task), priority);
}

void MapThreadManager::create_map_thread(int map_id, const std::string& map_name) {
    // 구현
}
```

## 📋 네이밍 컨벤션

### **클래스/구조체**
```cpp
// ✅ PascalCase
class GameServer;
struct Player;
enum class GameObjectType;

// ❌ snake_case
class game_server;
struct player;
```

### **함수/변수**
```cpp
// ✅ snake_case
void update_ecs(float delta_time);
int player_count;
std::string player_name;

// ❌ camelCase
void updateEcs(float deltaTime);
int playerCount;
std::string playerName;
```

### **상수/매크로**
```cpp
// ✅ UPPER_CASE
const int MAX_PLAYERS = 1000;
#define LOGI(...) do { /* 구현 */ } while(0)

// ❌ mixed case
const int maxPlayers = 1000;
#define LogInfo(...) do { /* 구현 */ } while(0)
```

### **네임스페이스**
```cpp
// ✅ snake_case
namespace lemondory::game;
namespace lemondory::network;
namespace lemondory::protocol;

// ❌ PascalCase
namespace Lemondory::Game;
```

## 🔧 자동 포맷팅 도구

### **Clang-Format 설정**
```yaml
# .clang-format
BasedOnStyle: Google
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
BreakBeforeBraces: Attach
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: Never
AllowShortLoopsOnASingleLine: false
```

### **VSCode 설정**
```json
{
    "C_Cpp.clang_format_style": "Google",
    "editor.formatOnSave": true,
    "editor.formatOnPaste": true
}
```

## 📊 코드 품질 체크리스트

### **스타일 일관성**
- [ ] 중괄호 스타일 통일 (K&R)
- [ ] 들여쓰기 통일 (4 spaces)
- [ ] 네이밍 컨벤션 준수
- [ ] 줄 길이 제한 (100자)

### **가독성**
- [ ] 의미있는 변수명
- [ ] 적절한 주석
- [ ] 함수 길이 제한 (50줄)
- [ ] 복잡도 제한 (10 이하)

### **성능**
- [ ] 불필요한 복사 방지
- [ ] 이동 의미론 활용
- [ ] 인라인 함수 적절히 사용
- [ ] 메모리 할당 최적화

---

## 프로토콜 직렬화 체크리스트

패킷 struct를 추가하거나 수정할 때 반드시 확인.

### float 직렬화 (serialize)
```cpp
// ✅ 올바름 — write_float_le (memcpy 기반, 정렬 UB 없음)
write_float_le(out, x);

// ❌ 잘못됨 — reinterpret_cast: 정렬 UB, 플랫폼 의존
write_le32(out, *reinterpret_cast<const std::uint32_t*>(&x));
```

### float 역직렬화 (parse)
```cpp
// ✅ 올바름 — read_float_le (LE 디코딩 후 memcpy)
out.x = read_float_le(p); p += 4;

// ❌ 잘못됨 — reinterpret_cast: 정렬 UB, BE 플랫폼에서 값 반전
out.x = *reinterpret_cast<const float*>(p); p += 4;
```

### 단일 바이트 필드 (uint8_t, bool)
```cpp
// ✅ 올바름 — p를 정확히 1 전진
out.field = static_cast<std::uint8_t>(*p++);

// ❌ 잘못됨 — p를 2 전진시켜 다음 바이트를 건너뜀
out.field = *p++; p += 1;  // BUG: double-increment
```

### 경계 검사 (parse 최소 크기)
```cpp
// 필드 크기를 직접 합산해서 검사값 결정
// le32=4, le16=2, bool/uint8=1, lp_string=최소 4(len)+N
// 예: le32(4) + uint8(1) + le32(4) + le32(4) + le32(4) = 17
if (p + 17 > end) return false;

// ❌ 의심스러운 경우: 옛날 숫자를 복붙하지 말 것
if (p + 20 > end) return false;  // 실제 최소가 21이라면 버그
```

### 핸들러에서 float 읽기
```cpp
// ✅ protocol struct::parse 사용
player_move_req req;
if (!player_move_req::parse(data, data + size, req)) return;
float x = req.x;  // 정확한 LE 디코딩 보장

// ❌ raw reinterpret_cast — 정렬 UB
float x = *reinterpret_cast<const float*>(data);

// ❌ raw memcpy — LE 보장 없음 (BE 환경 불안)
float x; std::memcpy(&x, data, 4);
```

### 브로드캐스트 버퍼 구성
```cpp
// ✅ write_le32 / write_float_le 사용
std::vector<char> buf;
write_le32(buf, static_cast<std::uint32_t>(channel_id));
write_float_le(buf, x);

// ❌ memcpy(&channel_id, 4) — 네이티브 엔디안 그대로 복사
std::memcpy(buf.data(), &channel_id, 4);
```

---

**📝 이 스타일 가이드는 프로젝트 전체의 코드 일관성을 보장하기 위한 규칙입니다.**
