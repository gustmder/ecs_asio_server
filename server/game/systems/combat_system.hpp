#pragma once

#include "../system.hpp"
#include "../entity.hpp"
#include "../components/health.hpp"
#include <cstdint>

namespace lemondory::game {

// 전투 시스템 - 전투 로직
class CombatSystem : public System {
public:
    void update(float delta_time) override;
    
    // 전투 처리
    void attack(Entity attacker, Entity target);
    void take_damage(Entity entity, std::uint32_t damage);
    void heal(Entity entity, std::uint32_t amount);
    
    // 전투 상태 조회
    bool is_in_combat(Entity entity);
Entity get_target(Entity entity);
    std::uint32_t get_hp(Entity entity);
    std::uint32_t get_max_hp(Entity entity);
    
private:
    void process_attacks(float delta_time);
    void update_combat_states(float delta_time);
    void handle_death(Entity entity);
    void calculate_damage(Entity attacker, Entity target);
};

} // namespace lemondory::game
