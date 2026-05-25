#include "movement_system.hpp"
#include "../ecs/component.hpp"
#include "../components/transform.hpp"
#include "../components/game_object.hpp"
#include "../ecs/game_service.hpp"
#include <algorithm>
#include <cmath>

namespace lemondory::game {

// MovementSystem 구현
void MovementSystem::update(float delta_time) {
    update_positions(delta_time);
    check_boundaries();
    handle_collisions();
}

void MovementSystem::move_entity(Entity entity, float delta_x, float delta_y, float delta_z) {
    auto* position = component_manager().get_component<Position>(entity);
    if (position) {
        position->x += delta_x;
        position->y += delta_y;
        position->z += delta_z;
    }
}

void MovementSystem::set_velocity(Entity entity, float vx, float vy, float vz) {
    auto* velocity = component_manager().get_component<Velocity>(entity);
    if (velocity) {
        velocity->x = vx;
        velocity->y = vy;
        velocity->z = vz;
    }
}

void MovementSystem::stop_entity(Entity entity) {
    set_velocity(entity, 0.0f, 0.0f, 0.0f);
}

Position* MovementSystem::get_position(Entity entity) {
    return component_manager().get_component<Position>(entity);
}

Velocity* MovementSystem::get_velocity(Entity entity) {
    return component_manager().get_component<Velocity>(entity);
}

std::vector<Entity> MovementSystem::get_entities_in_range(float x, float y, float z, float range) {
    std::vector<Entity> entities_in_range;
    
    // 모든 Position 컴포넌트를 가진 엔티티 검사
    auto positions = component_manager().get_components<Position>();
    for (auto& [entity, position] : positions) {
        float dx = position->x - x;
        float dy = position->y - y;
        float dz = position->z - z;
        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        if (distance <= range) {
            entities_in_range.push_back(entity);
        }
    }
    
    return entities_in_range;
}

void MovementSystem::update_positions(float delta_time) {
    // 모든 Position + Velocity 컴포넌트를 가진 엔티티 업데이트
    auto positions = component_manager().get_components<Position>();
    for (auto& [entity, position] : positions) {
        auto* velocity = component_manager().get_component<Velocity>(entity);
        if (velocity) {
            position->x += velocity->x * delta_time;
            position->y += velocity->y * delta_time;
            position->z += velocity->z * delta_time;
        }
    }
}

void MovementSystem::check_boundaries() {
    // 맵 경계 검사 (임시로 -1000 ~ 1000 범위)
    const float MIN_BOUND = -1000.0f;
    const float MAX_BOUND = 1000.0f;
    
    auto positions = component_manager().get_components<Position>();
    for (auto& [entity, position] : positions) {
        // X 경계
        if (position->x < MIN_BOUND) position->x = MIN_BOUND;
        if (position->x > MAX_BOUND) position->x = MAX_BOUND;
        
        // Y 경계
        if (position->y < MIN_BOUND) position->y = MIN_BOUND;
        if (position->y > MAX_BOUND) position->y = MAX_BOUND;
        
        // Z 경계
        if (position->z < MIN_BOUND) position->z = MIN_BOUND;
        if (position->z > MAX_BOUND) position->z = MAX_BOUND;
    }
}

void MovementSystem::handle_collisions() {
    // 간단한 충돌 검사 (거리 기반)
    const float COLLISION_RADIUS = 1.0f;
    
    auto positions = component_manager().get_components<Position>();
    std::vector<std::pair<Entity, Position*>> entities;
    
    for (auto& [entity, position] : positions) {
        entities.push_back({entity, position});
    }
    
    // 모든 엔티티 쌍에 대해 충돌 검사
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            auto [entity1, pos1] = entities[i];
            auto [entity2, pos2] = entities[j];
            
            float dx = pos1->x - pos2->x;
            float dy = pos1->y - pos2->y;
            float dz = pos1->z - pos2->z;
            float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            if (distance < COLLISION_RADIUS * 2) {
                // 충돌 처리 (간단한 분리)
                float separation = (COLLISION_RADIUS * 2 - distance) / 2.0f;
                float dx_norm = dx / distance;
                float dy_norm = dy / distance;
                float dz_norm = dz / distance;
                
                pos1->x += dx_norm * separation;
                pos1->y += dy_norm * separation;
                pos1->z += dz_norm * separation;
                
                pos2->x -= dx_norm * separation;
                pos2->y -= dy_norm * separation;
                pos2->z -= dz_norm * separation;
            }
        }
    }
}

} // namespace lemondory::game
