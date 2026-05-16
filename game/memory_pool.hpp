#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <type_traits>

namespace lemondory::game::ecs {

// 메모리 풀 - 성능 최적화를 위한 객체 재사용
template<typename T>
class MemoryPool {
public:
    MemoryPool(size_t initial_size = 1000) : pool_size_(initial_size) {
        // 초기 메모리 할당
        for (size_t i = 0; i < initial_size; ++i) {
            free_objects_.push(std::make_unique<T>());
        }
    }
    
    ~MemoryPool() = default;
    
    // 객체 획득
    std::unique_ptr<T> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (free_objects_.empty()) {
            // 풀이 비어있으면 확장
            expand_pool();
        }
        
        auto obj = std::move(free_objects_.front());
        free_objects_.pop();
        return obj;
    }
    
    // 객체 반환
    void release(std::unique_ptr<T> obj) {
        if (!obj) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 객체 초기화 (재사용을 위해)
        *obj = T{};
        
        free_objects_.push(std::move(obj));
    }
    
    // 풀 크기 조정
    void reserve(size_t size) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        while (free_objects_.size() < size) {
            free_objects_.push(std::make_unique<T>());
        }
    }
    
    // 통계
    size_t get_free_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return free_objects_.size();
    }
    
    size_t get_total_count() const {
        return pool_size_;
    }
    
private:
    void expand_pool() {
        // 풀 크기를 2배로 확장
        size_t new_size = pool_size_ * 2;
        for (size_t i = pool_size_; i < new_size; ++i) {
            free_objects_.push(std::make_unique<T>());
        }
        pool_size_ = new_size;
    }
    
    std::queue<std::unique_ptr<T>> free_objects_;
    mutable std::mutex mutex_;
    size_t pool_size_;
};

// 컴포넌트 전용 메모리 풀
template<typename T>
class ComponentPool {
public:
    ComponentPool(size_t initial_size = 1000) : pool_(initial_size) {}
    
    // 컴포넌트 생성
    std::unique_ptr<T> create() {
        return pool_.acquire();
    }
    
    // 컴포넌트 반환
    void destroy(std::unique_ptr<T> component) {
        pool_.release(std::move(component));
    }
    
    // 통계
    size_t get_free_count() const { return pool_.get_free_count(); }
    size_t get_total_count() const { return pool_.get_total_count(); }
    
private:
    MemoryPool<T> pool_;
};

} // namespace lemondory::game::ecs
