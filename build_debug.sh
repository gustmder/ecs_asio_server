#!/bin/bash
# 디버그 빌드 스크립트

echo "Building debug_test..."

# 디버그 테스트 빌드
clang++ -std=c++17 \
    -I. -Icore -Inetwork -Igame \
    -I../asio/include -I../spdlog/include \
    -DASIO_STANDALONE \
    -g -O0 -Wall -Wextra \
    -o debug_test \
    tests/debug_test.cpp \
    core/packet_buffer.cpp

if [ $? -eq 0 ]; then
    echo "✅ debug_test built successfully!"
else
    echo "❌ Build failed!"
    exit 1
fi

echo "Building game_server..."

# 게임 서버 빌드 (unity_core.cpp 사용하지 않음)
clang++ -std=c++17 \
    -I. -Icore -Inetwork -Igame \
    -I../asio/include -I../spdlog/include \
    -DASIO_STANDALONE \
    -g -O0 -Wall -Wextra \
    -o game_server \
    game/main.cpp \
    game/game_server.cpp \
    core/packet_buffer.cpp \
    core/thread_group.cpp \
    network/socket_buffer.cpp \
    network/socket_channel_base.cpp \
    network/asio_channel.cpp \
    network/asio_server.cpp

if [ $? -eq 0 ]; then
    echo "✅ game_server built successfully!"
else
    echo "❌ Build failed!"
    exit 1
fi

echo "All builds completed!"