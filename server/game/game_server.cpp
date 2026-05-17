#include "game_server.hpp"
// #include "managers/object_manager.hpp"  // ECS로 대체
// #include "managers/map_manager.hpp"     // ECS로 대체
#include "common/log.hpp"
#include <cstring>

namespace lemondory::game {
using namespace lemondory::network;

bool game_server::init(asio::io_context& io,
                       const socket_option& opt,
                       const std::string& ip, int port)
{
    // ECS 시스템 초기화
    if (config_.use_ecs) {
        if (!game_service().initialize()) {
            LOGE("ECS game service init failed");
            return false;
        }
        LOGI("ECS system initialized");
        
        // 스레딩 시스템 초기화
        initialize_threading_system();
    }
    
    server_ = std::make_shared<asio_server>(io);
    if (!server_->init(this, opt, 0, 0)) {
        LOGE("Server init failed"); return false;
    }
    LOGI("Server listening on " + ip + ":" + std::to_string(port));

    // server_->set_on_frame([this](int channel_id, std::uint16_t cmd, const char* payload, std::size_t size) {
    //     this->on_message(channel_id, cmd, payload, static_cast<std::int32_t>(size));
    // });
    // server_ 생성 및 init 이후, accept/listen 이전에 연결하면 안전합니다.
    server_->set_on_frame(
    [this](int channel_id, std::uint16_t cmd, const char* data, std::size_t size)
    {
        // 공통 레이어 → 게임 핸들러 디스패처로 위임
        dispatcher_.dispatch(cmd, channel_id, data, size);
    });

    // 예시 핸들러 등록: 1=PING -> PONG(2) 응답
    dispatcher_.bind(1, [this](int channel_id, const char* /*data*/, std::size_t /*size*/) {
        socket_channel_base* ch = nullptr;
        { std::lock_guard<std::mutex> lk(mtx_);
          auto it = channels_.find(channel_id);
          if (it != channels_.end()) ch = it->second;
        }
        const char pong[] = "PONG";
        if (ch) send_frame(ch, /*msg_id*/2, pong, sizeof(pong)-1);
    });

    // echo 예시: 100=echo
    dispatcher_.bind(100, [this](int channel_id, const char* data, std::size_t size) {
        socket_channel_base* ch = nullptr;
        { std::lock_guard<std::mutex> lk(mtx_);
          auto it = channels_.find(channel_id);
          if (it != channels_.end()) ch = it->second;
        }
        if (ch) send_frame(ch, 100, data, size);
    });

    // 게임 메시지 핸들러들
    // 1001 = LOGIN
    dispatcher_.bind(1001, [this](int channel_id, const char* data, std::size_t size) {
        handle_login(channel_id, data, size);
    });
    
    // 1002 = MOVE
    dispatcher_.bind(1002, [this](int channel_id, const char* data, std::size_t size) {
        handle_move(channel_id, data, size);
    });
    
    // 1003 = CHAT
    dispatcher_.bind(1003, [this](int channel_id, const char* data, std::size_t size) {
        handle_chat(channel_id, data, size);
    });
    
    // 1004 = ATTACK
    dispatcher_.bind(1004, [this](int channel_id, const char* data, std::size_t size) {
        handle_attack(channel_id, data, size);
    });

    if (!server_->listen(ip, port)) {
        LOGE("Server listen failed on " + ip + ":" + std::to_string(port));
        return false;
    }
    return true;
}

void game_server::stop() {
    if (server_) { server_->stop(); server_.reset(); }
    
    // 스레딩 시스템 종료
    shutdown_threading_system();
    
    std::lock_guard<std::mutex> lk(mtx_);
    channels_.clear();
    // codecs_.clear();
}

void game_server::on_channel_init(socket_channel_base* channel, bool /*use_plugin*/) {
    LOGD("channel init id={}", channel->get_channel_id());
}

void game_server::on_channel_destroy(socket_channel_base* channel) {
    LOGD("channel destroy id={}", channel->get_channel_id());
}

void game_server::on_accept(int channel_id, socket_channel_base* channel) {
    std::lock_guard<std::mutex> lk(mtx_);
    channels_[channel_id] = channel;

    // 채널별 codec 생성 (프레임 완성 시 dispatcher로)
    // codecs_[channel_id] = std::make_unique<frame_codec>(
    //     [this, channel_id](std::uint16_t msg_id, const char* data, std::size_t size) 
    //     {
    //         dispatcher_.dispatch(msg_id, channel_id, data, size);
    //     }
    // );

    // 스레딩 시스템: 플레이어를 기본 맵에 추가
    if (main_thread_manager_) {
        update_player_map(channel_id, 1);
        LOGD("Player {} added to map 1 (Forest)", channel_id);
    }

    LOGI("accept: channel={}", channel_id);
}

void game_server::on_close(int channel_id, const void* /*error*/, close_function reason) {
    std::lock_guard<std::mutex> lk(mtx_);
    channels_.erase(channel_id);
    channel_to_entity_.erase(channel_id);
    
    // 스레딩 시스템: 플레이어 맵 정보 제거
    if (main_thread_manager_) {
        std::lock_guard<std::mutex> map_lock(player_map_mutex_);
        player_map_mapping_.erase(channel_id);
    }

    LOGI("close: channel={} reason={}", channel_id, static_cast<int>(reason));
}

void game_server::on_tick() {
    if (config_.use_ecs) {
        // ECS 시스템 업데이트 (60 FPS)
        game_service().update(0.016f);
    }
}

// void game_server::on_receive(int channel_id, const char* buffer, std::int32_t size) 
// {
//     std::unique_ptr<frame_codec>* pcodec = nullptr;
//     {
//         std::lock_guard<std::mutex> lk(mtx_);
//         auto it = codecs_.find(channel_id);
//         if (it != codecs_.end()) pcodec = &it->second;
//     }
//     if (!pcodec || !(*pcodec)) return;
//     (*pcodec)->on_bytes(buffer, static_cast<std::size_t>(size));
// }

void game_server::send_frame(socket_channel_base* ch,
                             std::uint16_t /*msg_id*/,
                             const char* data, std::size_t size) {
    auto pkt = lemondory::network::frame_codec::pack(
        /*cmd*/ to_u16(lemondory::network::command::echo),
        /*flags*/ 0,
        /*ver*/   1,
        /*seq*/   0,
        /*payload*/ std::string_view{data, size}
    );
    ch->send(pkt.data(), static_cast<int>(pkt.size()));
}

void game_server::send_test_frame(int channel_id, std::uint16_t msg_id, const char* data, std::size_t size) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = channels_.find(channel_id);
    if (it != channels_.end()) {
        send_frame(it->second, msg_id, data, size);
    }
}

void game_server::broadcast(std::uint16_t cmd, const char* data, std::size_t size) {
    auto pkt = lemondory::network::frame_codec::pack(cmd, 0, 1, 0, std::string_view{data, size});
    std::lock_guard<std::mutex> lk(mtx_);
    for (auto& [id, ch] : channels_) {
        ch->send(pkt.data(), static_cast<int>(pkt.size()));
    }
}

void game_server::broadcast_except(int exclude_channel_id, std::uint16_t cmd, const char* data, std::size_t size) {
    auto pkt = lemondory::network::frame_codec::pack(cmd, 0, 1, 0, std::string_view{data, size});
    std::lock_guard<std::mutex> lk(mtx_);
    for (auto& [id, ch] : channels_) {
        if (id != exclude_channel_id) {
            ch->send(pkt.data(), static_cast<int>(pkt.size()));
        }
    }
}

// 게임 메시지 핸들러 구현들
void game_server::handle_login(int channel_id, const char* data, std::size_t size) {
    std::string player_name(data, size);
    LOGI("Player login: {} (channel={})", player_name, channel_id);

    if (config_.use_ecs) {
        Entity player = game_service().create_entity();

        game_service().add_component(player, std::make_unique<Position>(0.0f, 0.0f, 0.0f));
        game_service().add_component(player, std::make_unique<Velocity>(0.0f, 0.0f, 0.0f));
        game_service().add_component(player, std::make_unique<Player>(player_name, 1, channel_id));
        game_service().add_component(player, std::make_unique<Health>(100, 50));

        {
            std::lock_guard<std::mutex> lk(mtx_);
            channel_to_entity_[channel_id] = player;
        }

        LOGD("Player created via ECS (Entity={})", static_cast<int>(player));
    }
    
    const char* response = "LOGIN_SUCCESS";
    send_test_frame(channel_id, 1001, response, strlen(response));
}

void game_server::handle_move(int channel_id, const char* data, std::size_t size) {
    if (size < 12) {
        LOGW("Invalid move data from channel {}", channel_id);
        return;
    }
    
    // 스레딩 시스템: 패킷을 해당 맵 스레드로 라우팅
    if (main_thread_manager_) {
        route_packet_to_map(channel_id, 0x1001, data, size);
        return; // 맵 스레드에서 처리하므로 여기서는 종료
    }
    
    // 간단한 위치 파싱 (실제로는 프로토콜 버퍼 등 사용)
    float x = *reinterpret_cast<const float*>(data);
    float y = *reinterpret_cast<const float*>(data + 4);
    float z = *reinterpret_cast<const float*>(data + 8);
    
    LOGD("Player move: ({}, {}, {}) from channel={}", x, y, z, channel_id);

    if (config_.use_ecs) {
        Entity player_entity = 0;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            auto it = channel_to_entity_.find(channel_id);
            if (it != channel_to_entity_.end()) player_entity = it->second;
        }
        if (player_entity) {
            auto* position = game_service().get_component<Position>(player_entity);
            if (position) {
                position->x = x;
                position->y = y;
                position->z = z;
            }
        }
    }
    
    // 이동 결과를 다른 플레이어에게 브로드캐스트
    // payload: [channel_id(4)] [x(4)] [y(4)] [z(4)]
    char move_broadcast[16];
    std::memcpy(move_broadcast,      &channel_id, 4);
    std::memcpy(move_broadcast + 4,  &x, 4);
    std::memcpy(move_broadcast + 8,  &y, 4);
    std::memcpy(move_broadcast + 12, &z, 4);
    broadcast_except(channel_id, 1002, move_broadcast, sizeof(move_broadcast));

    const char* response = "MOVE_SUCCESS";
    send_test_frame(channel_id, 1002, response, strlen(response));
}

void game_server::handle_chat(int channel_id, const char* data, std::size_t size) {
    std::string message(data, size);
    LOGI("Chat from channel={}: {}", channel_id, message);
    
    // 스레딩 시스템: 글로벌 채팅은 메인 스레드에서 처리
    if (main_thread_manager_) {
        main_thread_manager_->handle_global_chat(channel_id, message);
        return;
    }
    
    // 채팅 메시지를 모든 플레이어에게 브로드캐스트
    // payload: [channel_id(4)] [message(variable)]
    std::vector<char> chat_broadcast(4 + message.size());
    std::memcpy(chat_broadcast.data(), &channel_id, 4);
    std::memcpy(chat_broadcast.data() + 4, message.data(), message.size());
    broadcast(1003, chat_broadcast.data(), chat_broadcast.size());

    const char* response = "CHAT_SUCCESS";
    send_test_frame(channel_id, 1003, response, strlen(response));
}

void game_server::handle_attack(int channel_id, const char* data, std::size_t size) {
    if (size < 4) {
        LOGW("Invalid attack data from channel {}", channel_id);
        return;
    }

    int target_id = *reinterpret_cast<const int*>(data);
    LOGI("Attack from channel={} to target={}", channel_id, target_id);
    
    // TODO: 실제 공격 로직 구현
    // - 타겟 검증
    // - 데미지 계산
    // - 결과 브로드캐스트
    
    const char* response = "ATTACK_SUCCESS";
    send_test_frame(channel_id, 1004, response, strlen(response));
}

// 스레딩 시스템 초기화
void game_server::initialize_threading_system() {
    main_thread_manager_ = std::make_unique<MainThreadManager>();
    main_thread_manager_->start();

    for (const auto& m : config_.maps)
        main_thread_manager_->create_map_thread(m.id, m.name);

    LOGI("Threading system initialized with {} map(s).", config_.maps.size());
}

// 스레딩 시스템 종료
void game_server::shutdown_threading_system() {
    if (main_thread_manager_) {
        LOGI("Shutting down threading system...");
        main_thread_manager_->stop();
        main_thread_manager_.reset();
        LOGI("Threading system shutdown complete.");
    }
}

// 플레이어가 어느 맵에 있는지 조회
int game_server::get_player_map(int channel_id) {
    std::lock_guard<std::mutex> lock(player_map_mutex_);
    auto it = player_map_mapping_.find(channel_id);
    return it != player_map_mapping_.end() ? it->second : 1; // 기본값: 맵 1
}

// 플레이어 맵 정보 업데이트
void game_server::update_player_map(int channel_id, int map_id) {
    std::lock_guard<std::mutex> lock(player_map_mutex_);
    player_map_mapping_[channel_id] = map_id;
}

// 패킷을 해당 맵 스레드로 라우팅
void game_server::route_packet_to_map(int channel_id, std::uint16_t cmd, const char* data, std::size_t size) {
    if (!main_thread_manager_) return;
    
    int map_id = get_player_map(channel_id);
    main_thread_manager_->send_packet_to_map(map_id, channel_id, cmd, data, size);
}

} // namespace lemondory::game