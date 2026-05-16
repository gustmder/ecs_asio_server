#pragma once

#include <cstdint>
#include "close_function.hpp"

namespace lemondory::network {
    
    struct message_meta {
        std::uint8_t  flags{};
        std::uint8_t  ver{1};
        std::uint32_t seq{};
        std::uint32_t crc{};
    };

    class socket_channel_base; // forward declaration
    enum class net_error;
    //enum class close_function;

    class net_handler {
    public:
        virtual ~net_handler() = default;

        virtual void on_channel_init(socket_channel_base* channel, bool use_plugin) = 0;
        virtual void on_channel_destroy(socket_channel_base* channel) = 0;

        // Optional events — override if needed
        virtual void on_accept(int /*channel_id*/, socket_channel_base* /*channel*/) {}
        virtual void on_close(int /*channel_id*/, const void* /*error*/, close_function /*reason*/) {}
        virtual void on_connect(net_error /*error*/) {}
        //virtual void on_receive(int channel_id, const char* buffer, std::int32_t size) {}
        virtual void on_error(net_error /*error*/) {}
        virtual void on_flush(int /*flush_size*/, int /*error_code*/ = 0) {}
        virtual void on_tick() {}
        // on_message: framed message callback (msg_id + payload)
        // 프레이밍된 메시지 콜백 (메시지 ID + 페이로드)
        // Default: no-op. Length-prefixed 프레임을 사용할 때 on_receive 대신 이걸 오버라이드.
        virtual void on_message(int /*channel_id*/, std::uint16_t /*cmd*/, const char* /*payload*/, std::int32_t /*size*/) {}
        // 한국어: 확장 메타 정보가 포함된 메시지 콜백
        virtual void on_message_ex(int channel_id, std::uint16_t cmd, const char* payload, std::int32_t size, const message_meta& /*meta*/)
        {
            // 기본 구현: 메타를 무시하고 기존 on_message로 폴백
            on_message(channel_id, cmd, payload, size);
        }
    };

} // namespace net