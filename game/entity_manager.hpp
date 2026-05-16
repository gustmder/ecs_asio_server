#pragma once

#include "entity.hpp"
#include "memory_pool.hpp"
#include <unordered_set>
#include <vector>
#include <mutex>
#include <atomic>

namespace lemondory::game::ecs {

// Entity Manager - Entity 생명주기 관리
class EntityManager {
public:
    static EntityManager& instance();
    
    // Entity 생성/삭제
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
    
private:
    EntityManager() = default;
    
    std::unordered_set<Entity> active_entities_;
    mutable std::mutex entities_mutex_;
    std::atomic<std::uint32_t> next_entity_id_{1};
    
    // 메모리 풀 (Entity ID 재사용)
    MemoryPool<Entity> entity_pool_;
};

} // namespace lemondory::game::ecs
