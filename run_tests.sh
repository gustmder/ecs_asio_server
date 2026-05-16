#!/bin/bash

# Lemondory Game Server - 테스트 실행 스크립트
# 사용법: ./run_tests.sh [테스트명]

cd "$(dirname "$0")/build"

echo "=== Lemondory Game Server 테스트 실행 ==="
echo ""

# 빌드 확인
if [ ! -f "Makefile" ]; then
    echo "❌ 빌드 디렉토리가 없습니다. 먼저 빌드하세요:"
    echo "   cd .. && cmake -B build && cd build && make -j4"
    exit 1
fi

# 테스트 목록 (bash 3.x 호환)
tests_integration="integration_test"
tests_network="test_network_stability"
tests_packet="test_packet_buffer"
tests_threading="test_threading_system"
tests_simple="simple_threading_test"
tests_benchmark="benchmark_performance"
tests_advanced="advanced_threading_test"

# 특정 테스트 실행
if [ $# -eq 1 ]; then
    test_name=$1
    test_file=""
    
    case $test_name in
        "integration") test_file="$tests_integration" ;;
        "network") test_file="$tests_network" ;;
        "packet") test_file="$tests_packet" ;;
        "threading") test_file="$tests_threading" ;;
        "simple") test_file="$tests_simple" ;;
        "benchmark") test_file="$tests_benchmark" ;;
        "advanced") test_file="$tests_advanced" ;;
        *) 
            echo "❌ 알 수 없는 테스트: $test_name"
            echo "사용 가능한 테스트: integration, network, packet, threading, simple, benchmark, advanced"
            exit 1
            ;;
    esac
    
    echo "🚀 실행 중: $test_name 테스트"
    echo "실행 파일: $test_file"
    echo ""
    
    if [ -f "$test_file" ]; then
        ./$test_file
        echo ""
        echo "✅ $test_name 테스트 완료"
    else
        echo "❌ 테스트 파일이 없습니다: $test_file"
        echo "먼저 빌드하세요: make $test_file"
    fi
    exit 0
fi

# 모든 테스트 실행
echo "🧪 모든 테스트 실행 중..."
echo ""

# 1. 통합 테스트 (핵심)
echo "1️⃣ 통합 테스트 (핵심 기능)"
if [ -f "integration_test" ]; then
    ./integration_test
    echo "✅ 통합 테스트 완료"
else
    echo "❌ integration_test 없음"
fi
echo ""

# 2. 네트워크 안정성 테스트
echo "2️⃣ 네트워크 안정성 테스트"
if [ -f "test_network_stability" ]; then
    ./test_network_stability
    echo "✅ 네트워크 테스트 완료"
else
    echo "❌ test_network_stability 없음"
fi
echo ""

# 3. 패킷 버퍼 테스트
echo "3️⃣ 패킷 버퍼 테스트"
if [ -f "test_packet_buffer" ]; then
    ./test_packet_buffer
    echo "✅ 패킷 버퍼 테스트 완료"
else
    echo "❌ test_packet_buffer 없음"
fi
echo ""

# 4. 스레딩 시스템 테스트
echo "4️⃣ 스레딩 시스템 테스트"
if [ -f "test_threading_system" ]; then
    ./test_threading_system
    echo "✅ 스레딩 테스트 완료"
else
    echo "❌ test_threading_system 없음"
fi
echo ""

# 5. 성능 벤치마크
echo "5️⃣ 성능 벤치마크"
if [ -f "benchmark_performance" ]; then
    ./benchmark_performance
    echo "✅ 성능 벤치마크 완료"
else
    echo "❌ benchmark_performance 없음"
fi
echo ""

echo "🎉 모든 테스트 완료!"
echo ""
echo "📋 사용법:"
echo "  ./run_tests.sh                    # 모든 테스트 실행"
echo "  ./run_tests.sh integration        # 특정 테스트만 실행"
echo "  ./run_tests.sh network           # 네트워크 테스트만"
echo "  ./run_tests.sh threading         # 스레딩 테스트만"
echo "  ./run_tests.sh benchmark         # 성능 테스트만"
