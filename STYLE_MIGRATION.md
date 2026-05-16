# 🔄 코드 스타일 마이그레이션 가이드

## 📋 현재 상황 분석

### **기존 코드 스타일 (2번 스타일)**
```cpp
// ❌ 기존 스타일 (Java 스타일)
void function_name() 
{
    if (condition) 
    {
        // 실행
    }
    else 
    {
        // 실행
    }
}

class MyClass 
{
public:
    void method() 
    {
        // 구현
    }
};
```

### **목표 코드 스타일 (1번 스타일)**
```cpp
// ✅ 목표 스타일 (현대 C++ 표준)
void function_name() {
    if (condition) {
        // 실행
    } else {
        // 실행
    }
}

class MyClass {
public:
    void method() {
        // 구현
    }
};
```

## 🎯 마이그레이션 전략

### **1. 점진적 마이그레이션 (권장)**
- **새로 작성하는 코드**: 1번 스타일 적용
- **기존 코드**: 필요시에만 수정
- **핵심 파일**: 우선 수정

### **2. 자동 변환 도구 활용**
```bash
# Clang-Format으로 자동 변환
clang-format -i --style=Google *.cpp *.hpp

# 또는 VSCode에서
# Ctrl+Shift+P → "Format Document"
```

### **3. 수동 변환 우선순위**
1. **새로 추가하는 파일** (최우선)
2. **핵심 네트워크 파일** (asio_server.cpp, asio_channel.cpp)
3. **ECS 핵심 파일** (entity.hpp, component.hpp, system.hpp)
4. **게임 로직 파일** (game_server.cpp, game_service.cpp)
5. **기타 파일** (필요시)

## 📁 우선 수정 대상 파일

### **1. 네트워크 레이어 (최우선)**
- `network/asio_server.cpp`
- `network/asio_channel.cpp`
- `network/frame_codec.hpp`

### **2. ECS 레이어 (높은 우선순위)**
- `game/entity.hpp`
- `game/component.hpp`
- `game/system.hpp`
- `game/game_service.hpp`

### **3. 게임 로직 (중간 우선순위)**
- `game/game_server.cpp`
- `game/systems/` 디렉토리

### **4. 테스트 파일 (낮은 우선순위)**
- `tests/` 디렉토리 (필요시)

## 🔧 자동 변환 스크립트

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

### **일괄 변환 스크립트**
```bash
#!/bin/bash
# convert_style.sh

echo "🔄 코드 스타일 변환 시작..."

# 네트워크 레이어
clang-format -i --style=Google network/*.cpp network/*.hpp

# ECS 레이어
clang-format -i --style=Google game/entity.hpp game/component.hpp game/system.hpp
clang-format -i --style=Google game/game_service.hpp game/game_service.cpp

# 게임 로직
clang-format -i --style=Google game/game_server.cpp game/game_server.hpp

echo "✅ 코드 스타일 변환 완료!"
```

## 📊 마이그레이션 체크리스트

### **변환 전 체크리스트**
- [ ] 백업 생성
- [ ] Git 커밋 (변경 전 상태 저장)
- [ ] 테스트 실행 (변경 전 동작 확인)

### **변환 후 체크리스트**
- [ ] 컴파일 확인
- [ ] 테스트 실행
- [ ] 기능 동작 확인
- [ ] Git 커밋 (변경 후 상태 저장)

## 🚨 주의사항

### **변환 시 주의사항**
1. **템플릿 코드**: 주의 깊게 변환
2. **매크로 정의**: 수동 확인 필요
3. **주석**: 보존 확인
4. **의미 변경**: 절대 금지

### **롤백 계획**
```bash
# Git으로 롤백
git checkout HEAD~1 -- network/ game/

# 또는 특정 파일만 롤백
git checkout HEAD~1 -- network/asio_server.cpp
```

## 📈 마이그레이션 진행률

### **1단계: 핵심 파일 (1주)**
- [ ] 네트워크 레이어 변환
- [ ] ECS 레이어 변환
- [ ] 기본 테스트

### **2단계: 게임 로직 (1주)**
- [ ] 게임 서버 변환
- [ ] 시스템 파일 변환
- [ ] 통합 테스트

### **3단계: 정리 (1주)**
- [ ] 테스트 파일 변환
- [ ] 문서 업데이트
- [ ] 최종 검증

---

**📝 이 가이드는 점진적이고 안전한 코드 스타일 마이그레이션을 위한 전략입니다.**
