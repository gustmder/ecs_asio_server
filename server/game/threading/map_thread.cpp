#include "map_thread.hpp"
#include "../components/health.hpp"
#include "common/log.hpp"
#include <chrono>
#include <algorithm>

namespace lemondory::game {

MapThread::MapThread(int map_id, const std::string& map_name, 
                    float width, float height, int max_players)
    : map_id_(map_id), map_name_(map_name), width_(width), height_(height), 
      max_players_(max_players), game_service_(GameService::instance()) {
    
    // 맵 엔티티 생성
    map_entity_ = game_service_.create_entity();
    
    // 맵 컴포넌트 추가
    game_service_.add_component(map_entity_, 
        std::make_unique<MapComponent>(map_id, map_name, width, height, max_players));
    
    // 맵 오브젝트 관리 컴포넌트 추가
    game_service_.add_component(map_entity_, std::make_unique<MapObjectsComponent>());
    
    // 맵 경계 컴포넌트 추가
    game_service_.add_component(map_entity_, 
        std::make_unique<MapBoundsComponent>(0, 0, 0, width, height, 1000.0f));
    
    // 컴포넌트 참조 저장
    map_component_ = game_service_.get_component<MapComponent>(map_entity_);
    map_objects_component_ = game_service_.get_component<MapObjectsComponent>(map_entity_);
    map_bounds_component_ = game_service_.get_component<MapBoundsComponent>(map_entity_);
    
    LOGI("MapThread created: map={} ({})", map_id, map_name);
}

MapThread::~MapThread() {
    stop();
}

void MapThread::start() {
    if (running_) return;
    
    running_ = true;
    thread_ = std::thread(&MapThread::thread_loop, this);
    
    LOGI("MapThread started: map={}", map_id_);
}

void MapThread::stop() {
    if (!running_) return;
    
    running_ = false;
    packet_cv_.notify_all();
    task_cv_.notify_all();
    
    if (thread_.joinable()) {
        thread_.join();
    }
    
    LOGI("MapThread stopped: map={}", map_id_);
}

void MapThread::join() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void MapThread::add_packet(int channel_id, std::uint16_t cmd, 
                          const char* data, std::size_t size) {
    std::lock_guard<std::mutex> lock(packet_mutex_);
    packet_queue_.emplace(channel_id, cmd, data, size);
    packet_cv_.notify_one();
}

void MapThread::process_packets() {
    std::unique_lock<std::mutex> lock(packet_mutex_);
    while (!packet_queue_.empty()) {
        auto packet = packet_queue_.front();
        packet_queue_.pop();
        lock.unlock();
        
        process_single_packet(packet);
        
        lock.lock();
    }
}

void MapThread::add_task(std::function<void()> task, int priority) {
    std::lock_guard<std::mutex> lock(task_mutex_);
    task_queue_.emplace(std::move(task), priority);
    task_cv_.notify_one();
}

void MapThread::process_tasks() {
    std::unique_lock<std::mutex> lock(task_mutex_);
    while (!task_queue_.empty()) {
        auto task = task_queue_.top();
        task_queue_.pop();
        lock.unlock();
        
        task.task();
        
        lock.lock();
    }
}

void MapThread::add_player(int channel_id, Entity player_entity) {
    std::lock_guard<std::mutex> lock(objects_mutex_);
    local_players_.insert(channel_id);
    player_count_.store(static_cast<int>(local_players_.size()));
    
    if (map_objects_component_) {
        map_objects_component_->add_player(player_entity);
    }
    
    LOGD("MapThread map={}: player {} added", map_id_, channel_id);
}

void MapThread::remove_player(int channel_id) {
    std::lock_guard<std::mutex> lock(objects_mutex_);
    local_players_.erase(channel_id);
    player_count_.store(static_cast<int>(local_players_.size()));
    
    LOGD("MapThread map={}: player {} removed", map_id_, channel_id);
}

void MapThread::update_player(int channel_id, const std::string& data) {
    // 플레이어 업데이트 로직
    LOGD("MapThread map={}: player {} updated", map_id_, channel_id);
}

void MapThread::update_map(float delta_time) {
    // 맵 업데이트
    update_players(delta_time);
    update_monsters(delta_time);
    update_npcs(delta_time);
    update_items(delta_time);
    
    // 맵 이벤트 처리
    handle_spawn_events(delta_time);
    handle_combat_events(delta_time);
    handle_interaction_events(delta_time);
}

void MapThread::update_players(float delta_time) {
    std::lock_guard<std::mutex> lock(objects_mutex_);
    
    for (int channel_id : local_players_) {
        // 플레이어 업데이트 로직
        // (구체적인 구현은 필요에 따라 추가)
    }
}

void MapThread::update_monsters(float delta_time) {
    std::lock_guard<std::mutex> lock(objects_mutex_);
    
    for (Entity monster : local_monsters_) {
        update_object_ai(monster, delta_time);
        update_object_position(monster, delta_time);
        check_object_collisions(monster);
    }
}

void MapThread::update_npcs(float delta_time) {
    std::lock_guard<std::mutex> lock(objects_mutex_);
    
    for (Entity npc : local_npcs_) {
        update_object_ai(npc, delta_time);
        update_object_position(npc, delta_time);
        handle_object_interactions(npc);
    }
}

void MapThread::update_items(float delta_time) {
    std::lock_guard<std::mutex> lock(objects_mutex_);
    
    for (Entity item : local_items_) {
        // 아이템 업데이트 로직
        // (구체적인 구현은 필요에 따라 추가)
    }
}

void MapThread::handle_spawn_events(float delta_time) {
    // 스폰 이벤트 처리
    spawn_monsters(delta_time);
    spawn_items(delta_time);
}

void MapThread::handle_combat_events(float delta_time) {
    // 전투 이벤트 처리
    // (구체적인 구현은 필요에 따라 추가)
}

void MapThread::handle_interaction_events(float delta_time) {
    // 상호작용 이벤트 처리
    // (구체적인 구현은 필요에 따라 추가)
}

// 카운터 메서드들은 이제 헤더에서 inline으로 정의됨

void MapThread::thread_loop() {
    LOGD("MapThread loop started: map={}", map_id_);
    
    auto last_update = std::chrono::steady_clock::now();
    
    while (running_) {
        auto now = std::chrono::steady_clock::now();
        float delta_time = std::chrono::duration<float>(now - last_update).count();
        last_update = now;
        
        // 1. 패킷 처리
        process_packets();
        
        // 2. 작업 처리
        process_tasks();
        
        // 3. 맵 업데이트
        update_map(delta_time);
        
        // 4. 프레임 레이트 제한 (60 FPS)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    LOGD("MapThread loop ended: map={}", map_id_);
}

void MapThread::process_single_packet(const MapPacket& packet) {
    // 패킷 타입에 따른 처리
    switch (packet.cmd) {
        case 0x1001: // 이동
            handle_movement_packet(packet.channel_id, packet.data);
            break;
        case 0x1002: // 전투
            handle_combat_packet(packet.channel_id, packet.data);
            break;
        case 0x1003: // 상호작용
            handle_interaction_packet(packet.channel_id, packet.data);
            break;
        case 0x1004: // 채팅
            handle_chat_packet(packet.channel_id, packet.data);
            break;
        default:
            LOGW("MapThread map={}: unknown cmd={:#x}", map_id_, packet.cmd);
            break;
    }
}

void MapThread::handle_movement_packet(int channel_id, const std::string& data) {
    if (data.size() < 12) return;
    float x = *reinterpret_cast<const float*>(data.data());
    float y = *reinterpret_cast<const float*>(data.data() + 4);
    float z = *reinterpret_cast<const float*>(data.data() + 8);
    LOGD("MapThread map={}: move channel={} ({:.1f},{:.1f},{:.1f})", map_id_, channel_id, x, y, z);
}

void MapThread::handle_combat_packet(int channel_id, const std::string& data) {
    LOGD("MapThread map={}: combat from channel={}", map_id_, channel_id);
}

void MapThread::handle_interaction_packet(int channel_id, const std::string& data) {
    LOGD("MapThread map={}: interaction from channel={}", map_id_, channel_id);
}

void MapThread::handle_chat_packet(int channel_id, const std::string& data) {
    LOGD("MapThread map={}: chat from channel={}", map_id_, channel_id);
}

void MapThread::update_object_position(Entity entity, float delta_time) {
    // 오브젝트 위치 업데이트
    auto* position = game_service_.get_component<Position>(entity);
    auto* velocity = game_service_.get_component<Velocity>(entity);
    
    if (position && velocity) {
        position->x += velocity->x * delta_time;
        position->y += velocity->y * delta_time;
        position->z += velocity->z * delta_time;
        
        // 맵 경계 검사
        if (!is_object_in_bounds(entity)) {
            clamp_object_to_bounds(entity);
        }
    }
}

void MapThread::update_object_ai(Entity entity, float delta_time) {
    // AI 업데이트
    // (구체적인 구현은 필요에 따라 추가)
}

void MapThread::check_object_collisions(Entity entity) {
    // 충돌 검사
    // (구체적인 구현은 필요에 따라 추가)
}

void MapThread::handle_object_interactions(Entity entity) {
    // 상호작용 처리
    // (구체적인 구현은 필요에 따라 추가)
}

bool MapThread::is_object_in_bounds(Entity entity) {
    if (!map_bounds_component_) return true;
    
    auto* position = game_service_.get_component<Position>(entity);
    if (!position) return true;
    
    return map_bounds_component_->is_inside(position->x, position->y, position->z);
}

void MapThread::clamp_object_to_bounds(Entity entity) {
    if (!map_bounds_component_) return;
    
    auto* position = game_service_.get_component<Position>(entity);
    if (!position) return;
    
    position->x = std::clamp(position->x, map_bounds_component_->min_x, map_bounds_component_->max_x);
    position->y = std::clamp(position->y, map_bounds_component_->min_y, map_bounds_component_->max_y);
    position->z = std::clamp(position->z, map_bounds_component_->min_z, map_bounds_component_->max_z);
}

void MapThread::spawn_monsters(float delta_time) {
    spawn_timer_ += delta_time;
    if (spawn_timer_ < SPAWN_INTERVAL) return;
    spawn_timer_ = 0.0f;

    std::lock_guard<std::mutex> lock(objects_mutex_);
    if (static_cast<int>(local_monsters_.size()) >= MAX_MONSTERS) return;

    // 맵 범위 내 랜덤 위치 계산 (간단한 선형 분포)
    static std::uint32_t seed = 42;
    auto next_rand = [&](float max) -> float {
        seed = seed * 1664525u + 1013904223u;
        return (seed % 10000) / 10000.0f * max;
    };

    int spawn_count = std::min(3, MAX_MONSTERS - static_cast<int>(local_monsters_.size()));
    for (int i = 0; i < spawn_count; ++i) {
        Entity entity = game_service_.create_entity();

        float x = next_rand(width_);
        float y = 0.0f;
        float z = next_rand(height_);

        game_service_.add_component(entity, std::make_unique<Position>(x, y, z));
        game_service_.add_component(entity, std::make_unique<Velocity>(0.0f, 0.0f, 0.0f));
        game_service_.add_component(entity, std::make_unique<Health>(100, 0));
        game_service_.add_component(entity, std::make_unique<Monster>(1, 1)); // type=1(Goblin), level=1
        game_service_.add_component(entity,
            std::make_unique<GameObject>(GameObjectType::MONSTER, "Goblin", entity));

        local_monsters_.insert(entity);
        monster_count_.store(static_cast<int>(local_monsters_.size()));

        if (map_objects_component_)
            map_objects_component_->add_monster(entity);
    }

    LOGD("Map {} spawned {} monsters (total={})", map_id_, spawn_count, local_monsters_.size());
}

void MapThread::spawn_items(float delta_time) {
    // 아이템 스폰 로직
    // (구체적인 구현은 필요에 따라 추가)
}

void MapThread::handle_map_events(float delta_time) {
    // 맵 이벤트 처리
    // (구체적인 구현은 필요에 따라 추가)
}

} // namespace lemondory::game
