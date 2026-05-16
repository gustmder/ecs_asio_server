#pragma once

#include "../system.hpp"
#include "../entity.hpp"
#include "../components/transform.hpp"
#include "../components/game_object.hpp"
#include "../components/health.hpp"
#include <vector>

namespace lemondory::game {

// AI 시스템 - 몬스터/NPC AI 로직
class AISystem : public System {
public:
    void update(float delta_time) override;
    
    // AI 상태 관리
    void set_target(Entity entity, Entity target);
    void clear_target(Entity entity);
    
    // AI 행동
    void move_towards_target(Entity entity, float speed);
    void patrol(Entity entity, float x, float y, float z, float range);
    void attack_target(Entity entity);
    
private:
    void update_monster_ai(float delta_time);
    void update_npc_ai(float delta_time);
    void find_nearest_target(Entity entity);
    void update_ai_state(Entity entity, float delta_time);
};

} // namespace lemondory::game
