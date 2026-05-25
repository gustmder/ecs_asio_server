#pragma once

#include "entity.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <mutex>

namespace lemondory::game {

// Component는 순수 데이터만 저장
class Component {
public:
    virtual ~Component() = default;
    virtual std::unique_ptr<Component> clone() const = 0;
};

// 타입 소거(type-erase)를 위한 추상 베이스 — ComponentManager가 unique_ptr로 소유
struct IComponentStore {
    virtual ~IComponentStore() = default;
};

// Component Store - 특정 타입의 컴포넌트들을 저장
template<typename T>
class ComponentStore : public IComponentStore {
public:
    void add(Entity entity, std::unique_ptr<T> component) {
        std::lock_guard<std::mutex> lock(mutex_);
        components_[entity] = std::move(component);
    }
    
    void remove(Entity entity) {
        std::lock_guard<std::mutex> lock(mutex_);
        components_.erase(entity);
    }
    
    T* get(Entity entity) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = components_.find(entity);
        return (it != components_.end()) ? it->second.get() : nullptr;
    }
    
    const T* get(Entity entity) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = components_.find(entity);
        return (it != components_.end()) ? it->second.get() : nullptr;
    }
    
    bool has(Entity entity) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return components_.count(entity);
    }

    // 락을 유지한 채 복사본 반환 — 시스템 이터레이션에 사용
    std::vector<std::pair<Entity, T*>> snapshot() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::pair<Entity, T*>> out;
        out.reserve(components_.size());
        for (auto& [e, c] : components_)
            out.emplace_back(e, c.get());
        return out;
    }

    std::vector<Entity> snapshot_entities() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Entity> out;
        out.reserve(components_.size());
        for (auto& [e, _] : components_)
            out.push_back(e);
        return out;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return components_.size();
    }

private:
    std::unordered_map<Entity, std::unique_ptr<T>> components_;
    mutable std::mutex mutex_;
};

// Component Manager - 모든 컴포넌트 타입을 관리
class ComponentManager {
public:
    static ComponentManager& instance();
    
    template<typename T>
    ComponentStore<T>& get_store() {
        std::lock_guard<std::mutex> lock(stores_mutex_);
        const std::type_index key = typeid(T);
        auto it = stores_.find(key);
        if (it == stores_.end()) {
            auto store = std::make_unique<ComponentStore<T>>();
            auto* raw = store.get();
            stores_.emplace(key, std::move(store));
            return *raw;
        }
        return *static_cast<ComponentStore<T>*>(it->second.get());
    }

    template<typename T>
    const ComponentStore<T>& get_store() const {
        std::lock_guard<std::mutex> lock(stores_mutex_);
        const std::type_index key = typeid(T);
        auto it = stores_.find(key);
        if (it == stores_.end()) {
            throw std::runtime_error("Component store not found");
        }
        return *static_cast<const ComponentStore<T>*>(it->second.get());
    }
    
    template<typename T>
    void add_component(Entity entity, std::unique_ptr<T> component) {
        get_store<T>().add(entity, std::move(component));
    }
    
    template<typename T>
    void remove_component(Entity entity) {
        get_store<T>().remove(entity);
    }
    
    template<typename T>
    T* get_component(Entity entity) {
        return get_store<T>().get(entity);
    }
    
    template<typename T>
    bool has_component(Entity entity) const {
        std::lock_guard<std::mutex> lock(stores_mutex_);
        auto it = stores_.find(std::type_index(typeid(T)));
        if (it == stores_.end()) return false;
        return static_cast<const ComponentStore<T>*>(it->second.get())->has(entity);
    }
    
    template<typename T>
    std::vector<Entity> get_entities_with_component() {
        return get_store<T>().snapshot_entities();
    }

    template<typename T>
    std::vector<std::pair<Entity, T*>> get_components() {
        return get_store<T>().snapshot();
    }
    
    // 엔티티 삭제 시 모든 컴포넌트 제거
    void remove_all_components(Entity entity);
    
    // 통계
    size_t get_component_count() const;
    size_t get_component_type_count() const;
    
public:
    ComponentManager() = default;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> stores_;
    mutable std::mutex stores_mutex_;
};

} // namespace lemondory::game
