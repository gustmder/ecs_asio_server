#pragma once
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>

#include "common/network/net_handler.hpp"
#include "common/network/socket_channel_base.hpp"
#include "server/network/asio_server.hpp"
#include "common/network/close_function.hpp"
#include "common/network/frame_codec.hpp"
#include "common/network/message_dispatcher.hpp"
#include "common/network/message_ids.hpp"
#include "game_service.hpp"
#include "components/transform.hpp"
#include "components/game_object.hpp"
#include "components/health.hpp"
#include "threading/main_thread_manager.hpp"

namespace lemondory::game {

class game_server : public lemondory::network::net_handler {
public:
    game_server() = default;
    ~game_server() override = default;

    bool init(asio::io_context& io,
              const lemondory::network::socket_option& opt,
              const std::string& ip, int port);
    void stop();

    // net_handler 구현
    void on_channel_init(lemondory::network::socket_channel_base* channel, bool use_plugin) override;
    void on_channel_destroy(lemondory::network::socket_channel_base* channel) override;
    void on_accept(int channel_id, lemondory::network::socket_channel_base* channel) override;
    void on_close(int channel_id, const void* error, lemondory::network::close_function reason) override;
    //void on_receive(int channel_id, const char* buffer, std::int32_t size) override;
    void on_tick() override;
    void on_message(int channel_id, std::uint16_t cmd, const char* payload, std::int32_t size) override
    {
        dispatcher_.dispatch(cmd, channel_id, payload, static_cast<std::size_t>(size));
    }

    //using msg_handler = std::function<void(int, std::string_view)>;

    // 메시지 핸들러 등록/해제.
    // void register_handler(std::uint16_t msg_id, msg_handler_t fn);
    // void unregister_handler(std::uint16_t msg_id);
    
    // 테스트용 public 메서드
    void send_test_frame(int channel_id, std::uint16_t msg_id, const char* data, std::size_t size);

    // 브로드캐스트 — 연결된 모든/일부 채널에 동일 프레임 전송
    void broadcast(std::uint16_t cmd, const char* data, std::size_t size);
    void broadcast_except(int exclude_channel_id, std::uint16_t cmd, const char* data, std::size_t size);
    
private:
    // 헬퍼
    void send_frame(lemondory::network::socket_channel_base* ch,
                    std::uint16_t msg_id,
                    const char* data, std::size_t size);
    
    // 게임 메시지 핸들러들
    void handle_login(int channel_id, const char* data, std::size_t size);
    void handle_move(int channel_id, const char* data, std::size_t size);
    void handle_chat(int channel_id, const char* data, std::size_t size);
    void handle_attack(int channel_id, const char* data, std::size_t size);

private:
    std::shared_ptr<lemondory::network::asio_server> server_;
    std::mutex mtx_;
    std::unordered_map<int, lemondory::network::socket_channel_base*> channels_;
    std::unordered_map<int, Entity> channel_to_entity_;   // channel_id → ECS Entity (O(1) lookup)

    // 채널별 프레이밍 상태
    //std::unordered_map<int, std::unique_ptr<lemondory::network::frame_codec>> codecs_;

    // message dispatcher
    lemondory::network::message_dispatcher dispatcher_;
    
    // ECS 기반 게임 서비스
    bool use_ecs_system_{true};
    
    // 스레딩 시스템
    std::unique_ptr<MainThreadManager> main_thread_manager_;
    
    // 플레이어 맵 매핑 (channel_id -> map_id)
    std::unordered_map<int, int> player_map_mapping_;
    std::mutex player_map_mutex_;
    
    // 스레딩 시스템 헬퍼
    void initialize_threading_system();
    void shutdown_threading_system();
    int get_player_map(int channel_id);
    void update_player_map(int channel_id, int map_id);
    void route_packet_to_map(int channel_id, std::uint16_t cmd, const char* data, std::size_t size);
};

} // namespace lemondory::game