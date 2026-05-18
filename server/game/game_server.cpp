#include "game_server.hpp"
#include "common/log.hpp"
#include "common/protocol/protocol_combat.hpp"
#include "common/protocol/protocol_map.hpp"
#include "common/protocol/protocol_guild.hpp"
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

    // 1005 = MAP_ENTER
    dispatcher_.bind(1005, [this](int channel_id, const char* data, std::size_t size) {
        handle_map_enter(channel_id, data, size);
    });

    // 2001 = GUILD_CREATE
    dispatcher_.bind(2001, [this](int channel_id, const char* data, std::size_t size) {
        handle_guild_create(channel_id, data, size);
    });
    // 2002 = GUILD_JOIN
    dispatcher_.bind(2002, [this](int channel_id, const char* data, std::size_t size) {
        handle_guild_join(channel_id, data, size);
    });
    // 2003 = GUILD_LEAVE
    dispatcher_.bind(2003, [this](int channel_id, const char* data, std::size_t size) {
        handle_guild_leave(channel_id, data, size);
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
    lemondory::shared::attack_req req;
    if (!lemondory::shared::attack_req::parse(data, data + size, req)) {
        LOGW("Invalid attack_req from channel={}", channel_id);
        return;
    }

    // target_id를 channel_id로 취급 (클라이언트가 상대 channel_id를 알고 있다고 가정)
    int target_channel = static_cast<int>(req.target_id);
    LOGI("Attack channel={} -> target_channel={} skill={}", channel_id, target_channel, req.skill_id);

    // 공격자 / 타겟 엔티티 조회
    Entity attacker_entity = 0, target_entity = 0;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (auto it = channel_to_entity_.find(channel_id); it != channel_to_entity_.end())
            attacker_entity = it->second;
        if (auto it = channel_to_entity_.find(target_channel); it != channel_to_entity_.end())
            target_entity = it->second;
    }

    if (!attacker_entity || !target_entity) {
        LOGW("Attack: entity not found (attacker={} target={})", attacker_entity, target_entity);
        lemondory::shared::attack_res res{false, "target not found", 0, 0, 0};
        auto payload = res.serialize();
        send_test_frame(channel_id, 1004, payload.data(), payload.size());
        return;
    }

    // 데미지 계산 (skill_id 기반 확장 가능, 현재는 고정 10)
    std::uint32_t damage = 10 + req.skill_id * 5;
    auto* combat = game_service().get_system<CombatSystem>();
    if (!combat) return;
    combat->take_damage(target_entity, damage);

    std::uint32_t remaining_hp = combat->get_hp(target_entity);
    bool target_dead = (remaining_hp == 0);

    // 공격자에게 결과 응답
    lemondory::shared::attack_res res{true, target_dead ? "killed" : "hit", 1, damage, remaining_hp};
    auto res_payload = res.serialize();
    send_test_frame(channel_id, 1004, res_payload.data(), res_payload.size());

    // 맵 전체에 데미지 브로드캐스트
    lemondory::shared::damage_notify notify{
        static_cast<std::uint32_t>(channel_id),
        req.target_id, damage, remaining_hp, 1, req.timestamp
    };
    auto notify_payload = notify.serialize();
    broadcast_except(channel_id, 1004, notify_payload.data(), notify_payload.size());

    if (target_dead) {
        LOGI("Entity {} killed by channel={}", static_cast<int>(target_entity), channel_id);
        game_service().destroy_entity(target_entity);
        std::lock_guard<std::mutex> lk(mtx_);
        channel_to_entity_.erase(target_channel);
    }
}

void game_server::handle_map_enter(int channel_id, const char* data, std::size_t size) {
    lemondory::shared::map_enter_req req;
    if (!lemondory::shared::map_enter_req::parse(data, data + size, req)) {
        LOGW("Invalid map_enter_req from channel={}", channel_id);
        return;
    }

    int target_map_id = static_cast<int>(req.map_id);
    LOGI("Map enter request: channel={} -> map={}", channel_id, target_map_id);

    // 요청한 맵이 설정에 존재하는지 확인
    const lemondory::core::MapConfig* map_cfg = nullptr;
    for (const auto& m : config_.maps) {
        if (m.id == target_map_id) { map_cfg = &m; break; }
    }

    if (!map_cfg) {
        LOGW("Map {} not found", target_map_id);
        lemondory::shared::map_enter_res res{false, "map not found", 0, "", 0, 0, 0, 0};
        auto payload = res.serialize();
        send_test_frame(channel_id, 1005, payload.data(), payload.size());
        return;
    }

    if (main_thread_manager_) {
        int current_map = get_player_map(channel_id);
        if (current_map != target_map_id)
            update_player_map(channel_id, target_map_id);
    }

    // 플레이어 엔티티의 map_id 업데이트
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (auto it = channel_to_entity_.find(channel_id); it != channel_to_entity_.end()) {
            if (auto* player = game_service().get_component<Player>(it->second))
                player->map_id = static_cast<std::uint32_t>(target_map_id);
        }
    }

    lemondory::shared::map_enter_res res{
        true, "ok",
        req.map_id, map_cfg->name,
        req.x, req.y, req.z,
        0
    };
    auto payload = res.serialize();
    send_test_frame(channel_id, 1005, payload.data(), payload.size());
    LOGI("channel={} entered map={} ({})", channel_id, target_map_id, map_cfg->name);
}

// ── 길드 핸들러 (인메모리, DB 연동 전 임시 구현) ────────────────────────────

void game_server::handle_guild_create(int channel_id, const char* data, std::size_t size) {
    lemondory::shared::guild_create_req req;
    if (!lemondory::shared::guild_create_req::parse(data, data + size, req)) {
        LOGW("Invalid guild_create_req from channel={}", channel_id);
        return;
    }

    std::lock_guard<std::mutex> lk(guild_mutex_);

    // 이미 길드에 속해 있으면 거부
    if (channel_to_guild_.count(channel_id)) {
        lemondory::shared::guild_create_res res{false, "already in a guild", 0};
        auto payload = res.serialize();
        send_test_frame(channel_id, 2001, payload.data(), payload.size());
        return;
    }

    std::uint32_t guild_id = next_guild_id_++;
    GuildData guild{guild_id, req.guild_name, req.guild_description, channel_id, {channel_id}};
    guilds_[guild_id] = std::move(guild);
    channel_to_guild_[channel_id] = guild_id;

    LOGI("Guild created: id={} name='{}' by channel={}", guild_id, req.guild_name, channel_id);

    lemondory::shared::guild_create_res res{true, "ok", guild_id};
    auto payload = res.serialize();
    send_test_frame(channel_id, 2001, payload.data(), payload.size());
}

void game_server::handle_guild_join(int channel_id, const char* data, std::size_t size) {
    lemondory::shared::guild_join_req req;
    if (!lemondory::shared::guild_join_req::parse(data, data + size, req)) {
        LOGW("Invalid guild_join_req from channel={}", channel_id);
        return;
    }

    std::lock_guard<std::mutex> lk(guild_mutex_);

    auto git = guilds_.find(req.guild_id);
    if (git == guilds_.end()) {
        lemondory::shared::guild_join_res res{false, "guild not found", 0};
        auto payload = res.serialize();
        send_test_frame(channel_id, 2002, payload.data(), payload.size());
        return;
    }
    if (channel_to_guild_.count(channel_id)) {
        lemondory::shared::guild_join_res res{false, "already in a guild", 0};
        auto payload = res.serialize();
        send_test_frame(channel_id, 2002, payload.data(), payload.size());
        return;
    }

    git->second.members.push_back(channel_id);
    channel_to_guild_[channel_id] = req.guild_id;

    LOGI("channel={} joined guild={} ({})", channel_id, req.guild_id, git->second.name);

    lemondory::shared::guild_join_res res{true, "ok", 0};
    auto payload = res.serialize();
    send_test_frame(channel_id, 2002, payload.data(), payload.size());
}

void game_server::handle_guild_leave(int channel_id, const char* data, std::size_t size) {
    std::lock_guard<std::mutex> lk(guild_mutex_);

    auto cit = channel_to_guild_.find(channel_id);
    if (cit == channel_to_guild_.end()) {
        LOGW("channel={} not in any guild (leave)", channel_id);
        return;
    }

    std::uint32_t guild_id = cit->second;
    channel_to_guild_.erase(cit);

    auto git = guilds_.find(guild_id);
    if (git != guilds_.end()) {
        auto& members = git->second.members;
        members.erase(std::remove(members.begin(), members.end(), channel_id), members.end());
        if (members.empty()) {
            LOGI("Guild {} disbanded (no members)", guild_id);
            guilds_.erase(git);
        }
    }

    LOGI("channel={} left guild={}", channel_id, guild_id);

    lemondory::shared::guild_leave_res res{true, "ok"};
    auto payload = res.serialize();
    send_test_frame(channel_id, 2003, payload.data(), payload.size());
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