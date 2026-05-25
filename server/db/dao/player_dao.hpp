#pragma once
#include <string>
#include <optional>
#include <cstdint>

namespace lemondory::db {

class DbConnectionPool;

struct PlayerRecord {
    std::int64_t id{0};
    std::string  name;
    int          level{1};
    int          exp{0};
    int          hp{100};
    int          max_hp{100};
    float        pos_x{0.f};
    float        pos_y{0.f};
    float        pos_z{0.f};
    int          map_id{1};
};

class PlayerDao {
public:
    explicit PlayerDao(DbConnectionPool& pool);

    std::optional<PlayerRecord> find_by_name(const std::string& name);
    // INSERT IGNORE → SELECT: 단일 커넥션으로 중복 로그인 레이스 없이 처리.
    // nullopt = 재시도 후에도 실패 (호출자가 로그인 거부 처리)
    std::optional<PlayerRecord> find_or_create(const std::string& name);
    // false = 재시도 후에도 실패 (호출자가 FailureSink에 스냅샷 기록)
    bool save_position(std::int64_t id, float x, float y, float z, int map_id);
    bool save_stats(std::int64_t id, int level, int exp, int hp);

private:
    DbConnectionPool& pool_;
};

} // namespace lemondory::db
