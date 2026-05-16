#pragma once

#include "../system.hpp"
#include "../components/map.hpp"
#include "../components/transform.hpp"
#include "../components/game_object.hpp"
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>

namespace lemondory::game {

// 맵별 스레드 관리
struct MapThread {
    std::thread thread;
    std::atomic<bool> running{true};
    std::queue<std::function<void()>> task_queue;
    std::mutex task_mutex;
    std::condition_variable task_cv;
    
    Entity map_entity;
    std::vector<Entity> local_objects;  // 이 스레드에서 관리하는 오브젝트들
    
    MapThread(Entity map_entity) : map_entity(map_entity) {}
    
    void add_task(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(task_mutex);
        task_queue.push(task);
        task_cv.notify_one();
    }
    
    void stop() {
        running = false;
        task_cv.notify_all();
        if (thread.joinable()) {
            thread.join();
        }
    }
};

class ThreadedMapSystem : public System {
public:
    ThreadedMapSystem() = default;
    ~ThreadedMapSystem() override;
    
    void update(float delta_time) override;
    
    // 맵 관리 (스레드 생성)
    Entity create_map(int map_id, const std::string& map_name, 
                     float width, float height, int max_players = 100);
    void destroy_map(Entity map_entity);
    
    // 오브젝트 관리 (스레드별 처리)
    void add_object_to_map(Entity map_entity, Entity object_entity);
    void remove_object_from_map(Entity map_entity, Entity object_entity);
    
    // 스레드별 업데이트
    void update_map_thread(Entity map_entity, float delta_time);
    
    // 스레드 간 통신
    void send_packet_to_map(Entity map_entity, const std::string& packet);
    void broadcast_to_map(Entity map_entity, const std::string& message);
    
private:
    std::unordered_map<Entity, std::unique_ptr<MapThread>> map_threads_;
    std::mutex map_threads_mutex_;
    
    void start_map_thread(Entity map_entity);
    void stop_map_thread(Entity map_entity);
    
    // 스레드별 업데이트 로직
    void update_map_objects(Entity map_entity, float delta_time);
    void process_map_packets(Entity map_entity);
    void handle_map_events(Entity map_entity, float delta_time);
};

} // namespace lemondory::game

