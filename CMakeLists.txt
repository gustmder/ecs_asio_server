cmake_minimum_required(VERSION 3.20)
project(asio_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
  add_compile_options(-O0 -g -fno-omit-frame-pointer -fno-inline)
endif()

set(ASIO_INCLUDE    "${CMAKE_SOURCE_DIR}/third_party/asio/asio/include")
set(SPDLOG_INCLUDE  "${CMAKE_SOURCE_DIR}/third_party/spdlog/include")
set(JSON_INCLUDE    "${CMAKE_SOURCE_DIR}/third_party")

# 프로젝트 루트를 include root 로 사용 — 모든 #include 는 루트 기준 전체 경로
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${ASIO_INCLUDE}
  ${SPDLOG_INCLUDE}
  ${JSON_INCLUDE}
)

# ─── 공통 라이브러리 ─────────────────────────────────────────────────────────

add_library(lemondory_core
  common/core/packet_buffer.cpp
)

add_library(lemondory_network
  common/network/socket_channel_base.cpp
  server/network/asio_channel.cpp
  server/network/asio_server.cpp
  server/core/thread_group.cpp
)
target_compile_definitions(lemondory_network PRIVATE ASIO_STANDALONE)
target_link_libraries(lemondory_network PUBLIC lemondory_core)

# ─── 게임 서버 라이브러리 ────────────────────────────────────────────────────

add_library(lemondory_game
  server/game/entity.cpp
  server/game/component.cpp
  server/game/system.cpp
  server/game/game_service.cpp
  server/game/player.cpp
  server/game/game_server.cpp
  server/game/systems/movement_system.cpp
  server/game/systems/ai_system.cpp
  server/game/systems/combat_system.cpp
  server/game/systems/map_system.cpp
  server/game/threading/main_thread_manager.cpp
  server/game/threading/map_thread_manager.cpp
  server/game/threading/map_thread.cpp
)
target_link_libraries(lemondory_game PUBLIC lemondory_core lemondory_network)

# ─── 실행 파일 ───────────────────────────────────────────────────────────────

add_executable(game_server server/main.cpp)
target_link_libraries(game_server PRIVATE lemondory_game)

add_executable(simple_client client/simple_client.cpp)
target_compile_definitions(simple_client PRIVATE ASIO_STANDALONE)
target_link_libraries(simple_client PRIVATE lemondory_network)

# ─── 테스트 ──────────────────────────────────────────────────────────────────

add_executable(test_packet_buffer    tests/test_packet_buffer.cpp)
target_link_libraries(test_packet_buffer    PRIVATE lemondory_core)

add_executable(test_network_stability tests/test_network_stability.cpp)
target_link_libraries(test_network_stability PRIVATE lemondory_game)

add_executable(test_ecs_system        tests/test_ecs_system.cpp)
target_link_libraries(test_ecs_system         PRIVATE lemondory_game)

add_executable(test_threading_system  tests/test_threading_system.cpp)
target_link_libraries(test_threading_system   PRIVATE lemondory_game)

add_executable(advanced_threading_test tests/advanced_threading_test.cpp)
target_link_libraries(advanced_threading_test  PRIVATE lemondory_game)

add_executable(simple_threading_test  tests/simple_threading_test.cpp)
target_link_libraries(simple_threading_test   PRIVATE lemondory_core)

add_executable(benchmark_performance  tests/benchmark_performance.cpp)
target_link_libraries(benchmark_performance   PRIVATE lemondory_core lemondory_network)

add_executable(debug_test             tests/debug_test.cpp)
target_link_libraries(debug_test              PRIVATE lemondory_core)

add_executable(integration_test       tests/integration_test.cpp)
target_link_libraries(integration_test        PRIVATE lemondory_core)

enable_testing()
add_test(NAME packet_buffer_test    COMMAND test_packet_buffer)
add_test(NAME network_stability_test COMMAND test_network_stability)
