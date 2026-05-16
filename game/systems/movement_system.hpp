#pragma once

#include "../system.hpp"
#include "../entity.hpp"
#include "../components/transform.hpp"
#include <vector>

namespace lemondory::game {

// 이동 시스템 - 위치 업데이트 로직
class MovementSystem : public System {
public:
    void update(float delta_time) override;
    
    // 이동 처리
    void move_entity(Entity entity, float delta_x, float delta_y, float delta_z);
    void set_velocity(Entity entity, float vx, float vy, float vz);
    void stop_entity(Entity entity);
    
    // 위치 조회
    Position* get_position(Entity entity);
    Velocity* get_velocity(Entity entity);
    
    // 범위 내 엔티티 검색
    std::vector<Entity> get_entities_in_range(float x, float y, float z, float range);
    
private:
    void update_positions(float delta_time);
    void check_boundaries();
    void handle_collisions();
};

} // namespace lemondory::game
