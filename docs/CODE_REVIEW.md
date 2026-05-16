# 🔍 Lemondory Game Server - 코드 검토 가이드

## 🎯 중점 검토 영역 (우선순위별)

### **1. 핵심 네트워크 레이어** ⭐⭐⭐
**파일**: `network/asio_server.cpp`, `network/asio_channel.cpp`

#### **검토 포인트**:
- **연결 관리**: 클라이언트 연결/해제 처리
- **메모리 안전성**: 버퍼 오버플로우, 메모리 누수
- **에러 처리**: 네트워크 오류 복구
- **동시성**: 멀티스레드 환경에서의 안전성

#### **핵심 코드**:
```cpp
// asio_server.cpp - 연결 수락
void asio_server::on_accept(std::shared_ptr<asio_channel> channel) {
    // 연결 처리 로직 검토
}

// asio_channel.cpp - 패킷 처리
void asio_channel::on_message(std::uint16_t cmd, const char* data, std::size_t size) {
    // 패킷 처리 로직 검토
}
```

### **2. ECS 엔진** ⭐⭐⭐
**파일**: `game/entity.hpp`, `game/component.hpp`, `game/system.hpp`

#### **검토 포인트**:
- **Entity 관리**: 생성/삭제 성능, ID 충돌 방지
- **Component 시스템**: 메모리 관리, 타입 안전성
- **System 업데이트**: 순서, 성능, 스레드 안전성
- **템플릿 메타프로그래밍**: 컴파일 타임 검증

#### **핵심 코드**:
```cpp
// component.hpp - 컴포넌트 관리
template<typename T>
bool has_component(Entity entity) const {
    // 타입 안전성 검토
}

// system.hpp - 시스템 업데이트
void SystemManager::update_all(float delta_time) {
    // 업데이트 순서 검토
}
```

### **3. 스레딩 시스템** ⭐⭐
**파일**: `game/threading/main_thread_manager.cpp`, `game/threading/map_thread_manager.cpp`

#### **검토 포인트**:
- **스레드 간 통신**: 큐 기반 메시지 전달
- **데드락 방지**: 락 순서, 타임아웃
- **작업 분배**: 우선순위, 부하 분산
- **리소스 정리**: 스레드 종료 시 정리

#### **핵심 코드**:
```cpp
// main_thread_manager.cpp - 작업 큐
void MainThreadManager::add_task(std::function<void()> task, int priority) {
    // 스레드 안전성 검토
}

// map_thread_manager.cpp - 맵 스레드 관리
void MapThreadManager::create_map_thread(int map_id) {
    // 스레드 생성 로직 검토
}
```

### **4. 게임 서버 메인** ⭐⭐
**파일**: `game/game_server.cpp`, `game/main.cpp`

#### **검토 포인트**:
- **초기화 순서**: 의존성 관리
- **에러 처리**: 예외 상황 대응
- **리소스 관리**: 메모리, 파일 핸들
- **설정 관리**: 환경별 설정

#### **핵심 코드**:
```cpp
// game_server.cpp - 서버 초기화
bool game_server::init(asio::io_context& io, const socket_option& opt,
                      const std::string& ip, int port) {
    // 초기화 순서 검토
}

// main.cpp - 메인 루프
int main() {
    // 서버 생명주기 관리 검토
}
```

### **5. 게임 로직 시스템** ⭐
**파일**: `game/systems/` 디렉토리

#### **검토 포인트**:
- **MovementSystem**: 이동 검증, 충돌 감지
- **CombatSystem**: 전투 밸런스, 데미지 계산
- **AISystem**: AI 행동 패턴, 성능
- **MapSystem**: 맵 관리, 경계 처리

## 🔧 기술적 검토 체크리스트

### **메모리 관리**
- [ ] 스마트 포인터 사용 (`std::unique_ptr`, `std::shared_ptr`)
- [ ] RAII 패턴 적용
- [ ] 메모리 누수 검사 (Valgrind, AddressSanitizer)
- [ ] 버퍼 오버플로우 방지

### **스레드 안전성**
- [ ] 뮤텍스 사용 (`std::mutex`, `std::lock_guard`)
- [ ] 원자적 연산 (`std::atomic`)
- [ ] 데드락 방지 (락 순서, 타임아웃)
- [ ] 데이터 레이스 방지

### **에러 처리**
- [ ] 예외 안전성 (Exception Safety)
- [ ] 에러 코드 vs 예외
- [ ] 로깅 시스템 활용
- [ ] 복구 가능한 에러 처리

### **성능 최적화**
- [ ] 불필요한 복사 방지 (이동 의미론)
- [ ] 캐시 친화적 데이터 구조
- [ ] 인라인 함수 활용
- [ ] 컴파일러 최적화 플래그

## 🚨 일반적인 문제점

### **메모리 관련**
```cpp
// ❌ 잘못된 예
char* buffer = new char[1024];
// ... 사용 후 delete[] 누락

// ✅ 올바른 예
std::unique_ptr<char[]> buffer = std::make_unique<char[]>(1024);
// 자동 해제
```

### **스레드 안전성**
```cpp
// ❌ 잘못된 예
int counter = 0;
void increment() { ++counter; }  // 데이터 레이스

// ✅ 올바른 예
std::atomic<int> counter{0};
void increment() { counter.fetch_add(1); }
```

### **템플릿 메타프로그래밍**
```cpp
// ❌ 잘못된 예
template<typename T>
T* get_component(Entity entity) {
    return static_cast<T*>(stores_[typeid(T)]);  // 타입 안전성 부족
}

// ✅ 올바른 예
template<typename T>
T* get_component(Entity entity) {
    static_assert(std::is_base_of_v<Component, T>);
    return get_store<T>().get(entity);
}
```

## 📊 코드 품질 메트릭

### **복잡도 측정**
- **순환 복잡도**: 함수당 10 이하 권장
- **중첩 깊이**: 4단계 이하 권장
- **함수 길이**: 50줄 이하 권장
- **클래스 크기**: 500줄 이하 권장

### **의존성 관리**
- **순환 의존성**: 방지
- **인터페이스 분리**: 작은 인터페이스
- **의존성 역전**: 추상화에 의존
- **모듈화**: 응집도 높고 결합도 낮게

## 🔍 디버깅 도구

### **정적 분석**
```bash
# Clang Static Analyzer
clang --analyze *.cpp

# Cppcheck
cppcheck --enable=all --inconclusive .

# PVS-Studio (상용)
pvs-studio-analyzer trace -- make
```

### **동적 분석**
```bash
# Valgrind (메모리 누수)
valgrind --leak-check=full ./game_server

# AddressSanitizer
g++ -fsanitize=address -g *.cpp
./a.out

# ThreadSanitizer
g++ -fsanitize=thread -g *.cpp
./a.out
```

### **프로파일링**
```bash
# gprof
g++ -pg -g *.cpp
./a.out
gprof a.out gmon.out

# perf
perf record ./game_server
perf report
```

## 📝 코드 리뷰 체크리스트

### **기능성**
- [ ] 요구사항 충족
- [ ] 엣지 케이스 처리
- [ ] 에러 상황 대응
- [ ] 사용자 경험

### **성능**
- [ ] 시간 복잡도
- [ ] 공간 복잡도
- [ ] 메모리 사용량
- [ ] CPU 사용률

### **유지보수성**
- [ ] 코드 가독성
- [ ] 문서화
- [ ] 테스트 커버리지
- [ ] 리팩토링 용이성

### **보안**
- [ ] 입력 검증
- [ ] 버퍼 오버플로우
- [ ] 메모리 누수
- [ ] 권한 관리

---

**📝 이 문서는 코드 검토 시 중점적으로 살펴볼 영역과 체크리스트를 제공합니다.**
