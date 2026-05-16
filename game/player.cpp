#include "player.hpp"
#include <cmath>
#include <algorithm>

namespace lemondory::game {

    player::player(int player_id, const std::string& name)
        : player_id_(player_id)
        , name_(name)
        , state_(player_state::offline)
        , position_(0, 0, 0)
        , level_(1)
        , hp_(100)
        , max_hp_(100)
        , channel_id_(-1)
        , last_activity_(std::chrono::steady_clock::now())
    {
    }

    void player::move_to(const Vector3& new_position) {
        position_ = new_position;
        update_activity();
    }

    float player::distance_to(const Vector3& other) const {
        float dx = position_.x - other.x;
        float dy = position_.y - other.y;
        float dz = position_.z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

} // namespace lemondory::game
