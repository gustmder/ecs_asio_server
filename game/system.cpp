#include "system.hpp"
#include <algorithm>
#include <typeindex>

namespace lemondory::game {

// SystemManager 구현
SystemManager& SystemManager::instance() {
    static SystemManager instance;
    return instance;
}


void SystemManager::update_all(float delta_time) {
    std::lock_guard<std::mutex> lock(systems_mutex_);
    
    for (System* system : update_order_) {
        if (system && system->is_enabled()) {
            system->update(delta_time);
        }
    }
}

void SystemManager::initialize_all() {
    std::lock_guard<std::mutex> lock(systems_mutex_);
    
    for (auto& [type_index, system] : systems_) {
        if (system) {
            system->initialize();
        }
    }
}

void SystemManager::shutdown_all() {
    std::lock_guard<std::mutex> lock(systems_mutex_);
    
    for (auto& [type_index, system] : systems_) {
        if (system) {
            system->shutdown();
        }
    }
}

size_t SystemManager::get_system_count() const {
    std::lock_guard<std::mutex> lock(systems_mutex_);
    return systems_.size();
}

std::vector<std::string> SystemManager::get_system_names() const {
    std::lock_guard<std::mutex> lock(systems_mutex_);
    
    std::vector<std::string> names;
    for (const auto& [type_index, system] : systems_) {
        names.push_back(type_index.name());
    }
    
    return names;
}

void SystemManager::rebuild_update_order() {
    update_order_.clear();
    
    for (auto& [type_index, system] : systems_) {
        if (system) {
            update_order_.push_back(system.get());
        }
    }
    
    // 우선순위별 정렬
    std::sort(update_order_.begin(), update_order_.end(),
              [](const System* a, const System* b) {
                  return a->get_priority() < b->get_priority();
              });
}

} // namespace lemondory::game
