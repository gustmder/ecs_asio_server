#pragma once

#include "../ecs/system.hpp"
#include "../components/map.hpp"
#include "../components/transform.hpp"
#include "../components/game_object.hpp"
#include <unordered_map>
#include <vector>

namespace lemondory::game {

class MapSystem : public System {
public:
    MapSystem() = default;
    ~MapSystem() override = default;
    
    void update(float delta_time) override;
    
    // 맵 관리
    Entity create_map(int map_id, const std::string& map_name, 
                     float width, float height, int max_players = 100);
    void destroy_map(Entity map_entity);
    
    // 오브젝트 관리
    void add_object_to_map(Entity map_entity, Entity object_entity);
    void remove_object_from_map(Entity map_entity, Entity object_entity);
    
    // 맵 검색
    std::vector<Entity> get_objects_in_map(Entity map_entity);
    std::vector<Entity> get_objects_in_range(Entity map_entity, float x, float y, float z, float range);
    
    // 맵 경계 검사
    bool is_object_in_bounds(Entity map_entity, Entity object_entity);
    void clamp_object_to_bounds(Entity map_entity, Entity object_entity);
    
    // 맵 통계
    int get_player_count(Entity map_entity);
    int get_monster_count(Entity map_entity);
    int get_total_object_count(Entity map_entity);
    
private:
    void update_map_objects(Entity map_entity, float delta_time);
    void check_map_collisions(Entity map_entity);
    void process_map_events(Entity map_entity, float delta_time);
    
    // 맵별 오브젝트 캐시 (성능 최적화)
    std::unordered_map<Entity, std::vector<Entity>> map_object_cache_;
    bool cache_dirty_ = true;
    
    void update_object_cache();
};

} // namespace lemondory::game

