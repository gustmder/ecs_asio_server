#pragma once

#include "../component.hpp"
#include <cstdint>

namespace lemondory::game {

// 위치 컴포넌트
struct Position : public Component {
    float x, y, z;
    
    Position(float x = 0.0f, float y = 0.0f, float z = 0.0f) 
        : x(x), y(y), z(z) {}
    
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Position>(x, y, z);
    }
};

// 회전 컴포넌트
struct Rotation : public Component {
    float x, y, z, w; // Quaternion
    
    Rotation(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f)
        : x(x), y(y), z(z), w(w) {}
    
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Rotation>(x, y, z, w);
    }
};

// 스케일 컴포넌트
struct Scale : public Component {
    float x, y, z;
    
    Scale(float x = 1.0f, float y = 1.0f, float z = 1.0f)
        : x(x), y(y), z(z) {}
    
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Scale>(x, y, z);
    }
};

// 속도 컴포넌트
struct Velocity : public Component {
    float x, y, z;
    
    Velocity(float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : x(x), y(y), z(z) {}
    
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Velocity>(x, y, z);
    }
};

} // namespace lemondory::game
