# 🧪 Lemondory Game Server - 테스트 가이드

## 📋 테스트 실행 명령어

### **전체 테스트 실행**
```bash
# 모든 테스트 실행
./run_tests.sh

# 특정 테스트만 실행
./run_tests.sh integration    # 통합 테스트
./run_tests.sh network       # 네트워크 테스트
./run_tests.sh threading     # 스레딩 테스트
./run_tests.sh benchmark     # 성능 테스트
```

### **개별 테스트 실행**
```bash
cd build

# 1. 통합 테스트 (핵심 기능)
./integration_test

# 2. 네트워크 안정성 테스트
./test_network_stability

# 3. 패킷 버퍼 테스트
./test_packet_buffer

# 4. 스레딩 시스템 테스트
./test_threading_system

# 5. 성능 벤치마크
./benchmark_performance

# 6. 고급 스레딩 테스트
./advanced_threading_test
```

## 🎯 테스트별 상세 설명

### **1. 통합 테스트 (integration_test)**
- **목적**: 전체 시스템 통합 동작 확인
- **검증 항목**:
  - ECS 시스템 초기화/종료
  - 스레드 생성/소멸
  - 메모리 안전성
  - 성능 벤치마크
- **실행 시간**: ~5초
- **중요도**: ⭐⭐⭐ (핵심)

### **2. 네트워크 안정성 테스트 (test_network_stability)**
- **목적**: 네트워크 연결 안정성 검증
- **검증 항목**:
  - 다중 클라이언트 연결
  - 패킷 송수신 정확성
  - 연결 해제 처리
  - 메모리 누수 방지
- **실행 시간**: ~10초
- **중요도**: ⭐⭐⭐ (핵심)

### **3. 패킷 버퍼 테스트 (test_packet_buffer)**
- **목적**: 패킷 처리 로직 검증
- **검증 항목**:
  - 버퍼 오버플로우 방지
  - 데이터 무결성
  - 메모리 관리
- **실행 시간**: ~2초
- **중요도**: ⭐⭐ (중요)

### **4. 스레딩 시스템 테스트 (test_threading_system)**
- **목적**: 멀티스레드 동작 검증
- **검증 항목**:
  - 스레드 간 통신
  - 데드락 방지
  - 작업 큐 관리
  - 동시성 안전성
- **실행 시간**: ~8초
- **중요도**: ⭐⭐ (중요)

### **5. 성능 벤치마크 (benchmark_performance)**
- **목적**: 성능 측정 및 최적화
- **검증 항목**:
  - 처리량 측정
  - 지연시간 측정
  - 메모리 사용량
  - CPU 사용률
- **실행 시간**: ~15초
- **중요도**: ⭐ (참고)

### **6. 고급 스레딩 테스트 (advanced_threading_test)**
- **목적**: 복잡한 스레딩 시나리오 검증
- **검증 항목**:
  - 동시 패킷 처리
  - 스트레스 테스트
  - 맵 스레드 격리
  - 성능 측정
- **실행 시간**: ~20초
- **중요도**: ⭐ (참고)

## 🔧 테스트 환경 설정

### **빌드 요구사항**
```bash
# CMake 3.20+
# C++17 컴파일러
# ASIO 라이브러리 (third_party 포함)
```

### **빌드 명령어**
```bash
# 전체 빌드
cmake -B build && cd build && make -j4

# 특정 테스트만 빌드
make integration_test
make test_network_stability
make benchmark_performance
```

## 📊 테스트 결과 해석

### **성공 기준**
- ✅ **통합 테스트**: 모든 기본 기능 동작
- ✅ **네트워크 테스트**: 연결 안정성 확인
- ✅ **스레딩 테스트**: 동시성 안전성 확인
- ✅ **성능 테스트**: 기준치 이상 성능

### **실패 시 대응**
- **컴파일 오류**: CMake 설정 확인
- **링크 오류**: 라이브러리 경로 확인
- **런타임 오류**: 로그 확인, 디버깅
- **성능 저하**: 프로파일링, 최적화

## 🚀 CI/CD 통합

### **자동화 스크립트**
```bash
#!/bin/bash
# ci_test.sh - CI/CD용 테스트 스크립트

set -e  # 오류 시 중단

echo "🔨 빌드 시작..."
cmake -B build
cd build
make -j4

echo "🧪 테스트 시작..."
./integration_test
./test_network_stability
./test_threading_system

echo "✅ 모든 테스트 통과!"
```

### **GitHub Actions 예시**
```yaml
name: Test Suite
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install cmake g++
      - name: Build
        run: cmake -B build && cd build && make -j4
      - name: Test
        run: ./run_tests.sh
```

## 📈 성능 모니터링

### **메트릭 수집**
- **처리량**: 초당 패킷 수
- **지연시간**: 평균/최대 응답 시간
- **메모리**: 힙 사용량, 누수 검사
- **CPU**: 스레드별 사용률

### **벤치마크 기준**
- **연결 수**: 1000+ 동시 연결
- **처리량**: 10000+ 패킷/초
- **지연시간**: <10ms 평균
- **메모리**: <100MB 기본 사용량

---

**📝 이 문서는 테스트 실행 방법과 결과 해석을 위한 가이드입니다.**
