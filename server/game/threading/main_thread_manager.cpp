#include "main_thread_manager.hpp"
#include <iostream>
#include <chrono>

namespace lemondory::game {

MainThreadManager::MainThreadManager() 
    : game_service_(GameService::instance()) {
    map_thread_manager_ = std::make_unique<MapThreadManager>();
}

MainThreadManager::~MainThreadManager() {
    stop();
}

void MainThreadManager::start() {
    if (running_) return;
    
    running_ = true;
    main_thread_ = std::thread(&MainThreadManager::main_loop, this);
    
    std::cout << "[MainThreadManager] Started main thread" << std::endl;
}

void MainThreadManager::stop() {
    if (!running_) return;
    
    running_ = false;
    task_cv_.notify_all();
    
    if (main_thread_.joinable()) {
        main_thread_.join();
    }
    
    // 모든 맵 스레드 정리
    if (map_thread_manager_) {
        map_thread_manager_->stop_all_map_threads();
    }
    
    std::cout << "[MainThreadManager] Stopped main thread" << std::endl;
}

void MainThreadManager::add_task(std::function<void()> task, int priority) {
    std::lock_guard<std::mutex> lock(task_mutex_);
    task_queue_.emplace(std::move(task), priority);
    task_cv_.notify_one();
}

void MainThreadManager::add_network_task(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(network_mutex_);
    network_tasks_.push(std::move(task));
}

void MainThreadManager::add_global_task(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(global_mutex_);
    global_tasks_.push(std::move(task));
}

void MainThreadManager::create_map_thread(int map_id, const std::string& map_name) {
    if (map_thread_manager_) {
        map_thread_manager_->create_map_thread(map_id, map_name, 1000.0f, 1000.0f, 100);
    }
}

void MainThreadManager::destroy_map_thread(int map_id) {
    if (map_thread_manager_) {
        map_thread_manager_->destroy_map_thread(map_id);
    }
}

void MainThreadManager::send_packet_to_map(int map_id, int channel_id, std::uint16_t cmd, 
                                         const char* data, std::size_t size) {
    if (map_thread_manager_) {
        map_thread_manager_->send_packet_to_map(map_id, channel_id, cmd, data, size);
    }
}

void MainThreadManager::handle_global_chat(int channel_id, const std::string& message) {
    add_global_task([this, channel_id, message]() {
        // 글로벌 채팅 처리
        std::cout << "[GlobalChat] Player " << channel_id << ": " << message << std::endl;
        
        // 모든 맵에 브로드캐스트
        auto active_maps = map_thread_manager_->get_active_map_ids();
        for (int map_id : active_maps) {
            map_thread_manager_->send_packet_to_map(map_id, channel_id, 0x1001, 
                                                  message.c_str(), message.size());
        }
    });
}

void MainThreadManager::handle_guild_system(int channel_id, const std::string& message) {
    add_global_task([channel_id, message]() {
        // 길드 시스템 처리
        std::cout << "[GuildSystem] Player " << channel_id << ": " << message << std::endl;
    });
}

void MainThreadManager::handle_friend_system(int channel_id, const std::string& message) {
    add_global_task([channel_id, message]() {
        // 친구 시스템 처리
        std::cout << "[FriendSystem] Player " << channel_id << ": " << message << std::endl;
    });
}

void MainThreadManager::handle_map_transfer(int channel_id, int from_map, int to_map) {
    add_task([this, channel_id, from_map, to_map]() {
        // 맵 이동 처리
        std::cout << "[MapTransfer] Player " << channel_id 
                  << " moving from map " << from_map << " to map " << to_map << std::endl;
        
        if (map_thread_manager_) {
            map_thread_manager_->move_player_to_map(channel_id, from_map, to_map);
        }
    }, 1); // 높은 우선순위
}

void MainThreadManager::update_ecs(float delta_time) {
    // ECS 업데이트
    game_service_.update(delta_time);
}

int MainThreadManager::get_map_thread_count() const {
    if (map_thread_manager_) {
        return map_thread_manager_->get_active_map_ids().size();
    }
    return 0;
}

void MainThreadManager::main_loop() {
    std::cout << "[MainThreadManager] Main loop started" << std::endl;
    
    auto last_update = std::chrono::steady_clock::now();
    
    while (running_) {
        auto now = std::chrono::steady_clock::now();
        float delta_time = std::chrono::duration<float>(now - last_update).count();
        last_update = now;
        
        // 1. 네트워크 이벤트 처리
        process_network_events();
        
        // 2. 글로벌 시스템 처리
        process_global_systems();
        
        // 3. 맵 이동 처리
        process_map_transfers();
        
        // 4. ECS 업데이트
        update_ecs(delta_time);
        
        // 5. 작업 큐 처리
        process_task_queue();
        
        // 6. 맵 스레드 업데이트
        if (map_thread_manager_) {
            map_thread_manager_->update_all_map_threads(delta_time);
        }
        
        // 7. 프레임 레이트 제한 (60 FPS)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    std::cout << "[MainThreadManager] Main loop ended" << std::endl;
}

void MainThreadManager::process_network_events() {
    std::lock_guard<std::mutex> lock(network_mutex_);
    while (!network_tasks_.empty()) {
        auto task = network_tasks_.front();
        network_tasks_.pop();
        task();
    }
}

void MainThreadManager::process_global_systems() {
    std::lock_guard<std::mutex> lock(global_mutex_);
    while (!global_tasks_.empty()) {
        auto task = global_tasks_.front();
        global_tasks_.pop();
        task();
    }
}

void MainThreadManager::process_map_transfers() {
    // 맵 이동 처리 로직
    // (구체적인 구현은 필요에 따라 추가)
}

void MainThreadManager::process_task_queue() {
    std::unique_lock<std::mutex> lock(task_mutex_);
    while (!task_queue_.empty()) {
        auto task = task_queue_.top();
        task_queue_.pop();
        lock.unlock();
        
        task.task();
        
        lock.lock();
    }
}

} // namespace lemondory::game
