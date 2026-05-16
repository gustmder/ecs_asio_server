#pragma once

#include <cstdint>
#include <atomic>
#include <unordered_set>
#include <vector>
#include <mutex>

namespace lemondory::game {

// Entity는 단순히 ID만 가진 태그
using Entity = std::uint32_t;

// Entity ID 생성기
class EntityManager {
public:
    static EntityManager& instance();
    
    Entity create_entity();
    void destroy_entity(Entity entity);
    bool is_valid(Entity entity) const;
    
    // Entity 검색
    std::vector<Entity> get_all_entities() const;
    std::vector<Entity> get_entities_in_range(float x, float y, float z, float range);
    
    // 통계
    std::uint32_t get_entity_count() const;
    std::uint32_t get_next_id() const;
    
    // 배치 작업
    void destroy_entities(const std::vector<Entity>& entities);
    std::vector<Entity> create_entities(std::uint32_t count);
    
public:
    EntityManager() = default;
    
    std::unordered_set<Entity> active_entities_;
    mutable std::mutex entities_mutex_;
    std::atomic<std::uint32_t> next_entity_id_{1};
    std::atomic<std::uint32_t> entity_count_{0};
};

} // namespace lemondory::game
