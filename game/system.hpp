#pragma once

#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace lemondory::game {

// System은 순수 로직만 처리 (상태 없음)
class System {
public:
    virtual ~System() = default;
    virtual void update(float delta_time) = 0;
    virtual void initialize() {}
    virtual void shutdown() {}
    
    // 시스템 활성화/비활성화
    void set_enabled(bool enabled) { enabled_ = enabled; }
    bool is_enabled() const { return enabled_; }
    
    // 업데이트 순서
    void set_priority(int priority) { priority_ = priority; }
    int get_priority() const { return priority_; }
    
protected:
    bool enabled_{true};
    int priority_{0};
};

// System Manager - 모든 시스템을 관리
class SystemManager {
public:
    static SystemManager& instance();
    
    template<typename T, typename... Args>
    T& register_system(Args&&... args) {
        static_assert(std::is_base_of<System, T>::value, "T must inherit from System");
        std::lock_guard<std::mutex> lock(systems_mutex_);
        
        std::type_index type_id = typeid(T);
        if (systems_.count(type_id)) {
            return *static_cast<T*>(systems_[type_id].get());
        }
        
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw_ptr = system.get();
        systems_[type_id] = std::move(system);
        rebuild_update_order();
        
        return *raw_ptr;
    }
    
    template<typename T>
    T* get_system() {
        std::lock_guard<std::mutex> lock(systems_mutex_);
        std::type_index type_id = typeid(T);
        auto it = systems_.find(type_id);
        return (it != systems_.end()) ? static_cast<T*>(it->second.get()) : nullptr;
    }
    
    template<typename T>
    void unregister_system() {
        std::lock_guard<std::mutex> lock(systems_mutex_);
        std::type_index type_id = typeid(T);
        systems_.erase(type_id);
        rebuild_update_order();
    }
    
    // 모든 시스템 업데이트
    void update_all(float delta_time);
    
    // 시스템 초기화/종료
    void initialize_all();
    void shutdown_all();
    
    // 통계
    size_t get_system_count() const;
    std::vector<std::string> get_system_names() const;
    
public:
    SystemManager() = default;
    std::unordered_map<std::type_index, std::unique_ptr<System>> systems_;
    std::vector<System*> update_order_;
    mutable std::mutex systems_mutex_;
    
    void rebuild_update_order();
};

} // namespace lemondory::game
