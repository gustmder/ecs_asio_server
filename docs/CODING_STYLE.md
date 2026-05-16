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

**📝 이 스타일 가이드는 프로젝트 전체의 코드 일관성을 보장하기 위한 규칙입니다.**
