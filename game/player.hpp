#pragma once

#include <string>
#include <memory>
#include <chrono>

namespace lemondory::game {

    // 3D 벡터 구조체
    struct Vector3 {
        float x, y, z;
        
        Vector3() : x(0), y(0), z(0) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
        
        bool operator==(const Vector3& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    // 플레이어 상태 열거형
    enum class player_state {
        offline,    // 오프라인
        online,     // 온라인 (로비)
        in_game,    // 게임 중
        in_room     // 룸 대기 중
    };

    // 플레이어 클래스
    class player {
    public:
        player(int player_id, const std::string& name);
        ~player() = default;

        // 기본 정보
        int get_player_id() const { return player_id_; }
        const std::string& get_name() const { return name_; }
        player_state get_state() const { return state_; }
        
        // 위치 정보
        const Vector3& get_position() const { return position_; }
        void set_position(const Vector3& pos) { position_ = pos; }
        
        // 게임 상태
        int get_level() const { return level_; }
        int get_hp() const { return hp_; }
        int get_max_hp() const { return max_hp_; }
        
        // 상태 변경
        void set_state(player_state state) { state_ = state; }
        void set_hp(int hp) { hp_ = std::min(hp, max_hp_); }
        
        // 연결 정보
        int get_channel_id() const { return channel_id_; }
        void set_channel_id(int channel_id) { channel_id_ = channel_id; }
        
        // 시간 정보
        std::chrono::steady_clock::time_point get_last_activity() const { return last_activity_; }
        void update_activity() { last_activity_ = std::chrono::steady_clock::now(); }
        
        // 유틸리티
        bool is_online() const { return state_ != player_state::offline; }
        bool is_in_game() const { return state_ == player_state::in_game; }
        
        // 이동 처리
        void move_to(const Vector3& new_position);
        float distance_to(const Vector3& other) const;

    private:
        int player_id_;
        std::string name_;
        player_state state_;
        Vector3 position_;
        int level_;
        int hp_;
        int max_hp_;
        int channel_id_;  // 네트워크 채널 ID
        
        std::chrono::steady_clock::time_point last_activity_;
    };

    using player_ptr = std::shared_ptr<player>;

} // namespace lemondory::game
