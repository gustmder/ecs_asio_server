#include "game_service.hpp"
#include "systems/movement_system.hpp"
#include "systems/ai_system.hpp"
#include "systems/combat_system.hpp"
#include "systems/map_system.hpp"
#include "../common/log.hpp"
#include <memory>

namespace lemondory::game {

GameService& GameService::instance() {
    static GameService instance;
    return instance;
}

bool GameService::initialize() {
    if (initialized_) return true;
    
    LOGI("Initializing ECS Game Service...");
    
    // 매니저들 초기화
    entity_manager_ = std::make_unique<EntityManager>();
    component_manager_ = std::make_unique<ComponentManager>();
    system_manager_ = std::make_unique<SystemManager>();
    
    // 기본 시스템 등록
    register_system<MovementSystem>();
    register_system<AISystem>();
    register_system<CombatSystem>();
    register_system<MapSystem>();
    
    // 시스템 초기화
    system_manager_->initialize_all();
    
    initialized_ = true;
    LOGI("ECS Game Service initialized successfully!");
    return true;
}

void GameService::shutdown() {
    if (!initialized_) return;
    
    LOGI("Shutting down ECS Game Service...");
    
    // 시스템 종료
    system_manager_->shutdown_all();
    
    // 매니저 종료
    shutdown_managers();
    
    initialized_ = false;
    LOGI("ECS Game Service shutdown complete!");
}

EntityManager& GameService::get_entity_manager() {
    return *entity_manager_;
}

ComponentManager& GameService::get_component_manager() {
    return *component_manager_;
}

SystemManager& GameService::get_system_manager() {
    return *system_manager_;
}

void GameService::update(float delta_time) {
    if (!initialized_) return;
    
    // 모든 시스템 업데이트
    system_manager_->update_all(delta_time);
}

Entity GameService::create_entity() {
    return entity_manager_->create_entity();
}

void GameService::destroy_entity(Entity entity) {
    // 모든 컴포넌트 제거
    component_manager_->remove_all_components(entity);
    
    // 엔티티 삭제
    entity_manager_->destroy_entity(entity);
}

bool GameService::is_valid(Entity entity) {
    return entity_manager_->is_valid(entity);
}


std::uint32_t GameService::get_entity_count() const {
    return entity_manager_->get_entity_count();
}

std::uint32_t GameService::get_component_count() const {
    return static_cast<std::uint32_t>(component_manager_->get_component_count());
}

std::uint32_t GameService::get_system_count() const {
    return static_cast<std::uint32_t>(system_manager_->get_system_count());
}

void GameService::initialize_managers() {
    // 이미 initialize()에서 초기화됨
}

void GameService::shutdown_managers() {
    system_manager_.reset();
    component_manager_.reset();
    entity_manager_.reset();
}

} // namespace lemondory::game
