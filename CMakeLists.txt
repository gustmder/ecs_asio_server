cmake_minimum_required(VERSION 3.20)
project(asio_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug 기본 (VSCode CMake Tools에서 바꿔도 됨)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# 디버깅 편의 플래그
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
  add_compile_options(-O0 -g -fno-omit-frame-pointer -fno-inline)
endif()

# ASIO 경로 (third_party 사용)
set(ASIO_INCLUDE "${CMAKE_SOURCE_DIR}/third_party/asio/asio/include")
set(SPDLOG_INCLUDE "${CMAKE_SOURCE_DIR}/third_party/spdlog/include")

# include 디렉토리
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_SOURCE_DIR}/network
  ${ASIO_INCLUDE}
  ${SPDLOG_INCLUDE}
)

# 코어 라이브러리
add_library(lemondory_core
  core/packet_buffer.cpp
  core/thread_group.cpp
  # 필요시 core/spin_lock.cpp 등 추가
)

# 네트워크 라이브러리
add_library(lemondory_network
  network/socket_buffer.cpp
  network/socket_channel_base.cpp
  network/asio_channel.cpp
  network/asio_server.cpp
)

# 필요 상수 정의
target_compile_definitions(lemondory_network PRIVATE ASIO_STANDALONE)

# 게임 로직 라이브러리 (ECS + 시스템 + 스레딩)
add_library(lemondory_game
  game/entity.cpp
  game/component.cpp
  game/system.cpp
  game/game_service.cpp
  game/player.cpp
  game/game_server.cpp
  game/systems/movement_system.cpp
  game/systems/ai_system.cpp
  game/systems/combat_system.cpp
  game/systems/map_system.cpp
  game/threading/main_thread_manager.cpp
  game/threading/map_thread_manager.cpp
  game/threading/map_thread.cpp
)

target_link_libraries(lemondory_game
  PUBLIC lemondory_core lemondory_network
)

# 서버 실행 파일
add_executable(game_server game/main.cpp)
target_link_libraries(game_server PRIVATE lemondory_game)

# 데모 클라이언트
add_executable(simple_client simple_client.cpp)
target_compile_definitions(simple_client PRIVATE ASIO_STANDALONE)
target_link_libraries(simple_client PRIVATE lemondory_network)

# 테스트 실행 파일들
add_executable(test_packet_buffer
  tests/test_packet_buffer.cpp
)

target_link_libraries(test_packet_buffer
  PRIVATE lemondory_core
)

add_executable(test_network_stability
  tests/test_network_stability.cpp
)

target_link_libraries(test_network_stability
  PRIVATE lemondory_game
)

add_executable(benchmark_performance
  tests/benchmark_performance.cpp
)

target_link_libraries(benchmark_performance
  PRIVATE lemondory_core lemondory_network
)

add_executable(debug_test
  tests/debug_test.cpp
)

target_link_libraries(debug_test
  PRIVATE lemondory_core
)

# ECS 기반 테스트
add_executable(test_ecs_system
  test_ecs_system.cpp
)

target_link_libraries(test_ecs_system
  PRIVATE lemondory_game
)

# 스레딩 시스템 테스트
add_executable(test_threading_system
  tests/test_threading_system.cpp
)

target_link_libraries(test_threading_system
  PRIVATE lemondory_game
)

# 간단한 스레딩 테스트
add_executable(simple_threading_test
  tests/simple_threading_test.cpp
)

target_link_libraries(simple_threading_test
  PRIVATE lemondory_core
)

# 고급 스레딩 테스트
add_executable(advanced_threading_test
  tests/advanced_threading_test.cpp
)

target_link_libraries(advanced_threading_test
  PRIVATE lemondory_game
)

# 통합 테스트
add_executable(integration_test
  tests/integration_test.cpp
)

target_link_libraries(integration_test
  PRIVATE lemondory_core
)


# 테스트 실행 (선택사항)
enable_testing()
add_test(NAME packet_buffer_test COMMAND test_packet_buffer)
add_test(NAME network_stability_test COMMAND test_network_stability)
