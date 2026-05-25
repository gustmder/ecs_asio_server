#pragma once
#include <string>
#include <cstdint>
#include <mutex>
#include <fstream>

namespace lemondory::db {

// DB 최종 실패 시 플레이어 상태를 JSON Lines 파일로 보존.
// 서버 재시작 또는 DB 복구 후 replay 스크립트로 재반영 가능.
struct PlayerSaveSnapshot {
    std::int64_t player_id{0};
    std::string  player_name;
    std::string  operation;    // "save_position" | "save_stats" | "find_or_create"
    float        pos_x{0.f}, pos_y{0.f}, pos_z{0.f};
    int          map_id{1};
    int          level{1}, exp{0}, hp{100};
    int          attempts{0};
    std::string  error_msg;
};

class FailureSink {
public:
    explicit FailureSink(const std::string& path = "logs/failed_saves.jsonl");
    ~FailureSink();

    void record(const PlayerSaveSnapshot& snap);

    FailureSink(const FailureSink&) = delete;
    FailureSink& operator=(const FailureSink&) = delete;

private:
    std::ofstream file_;
    std::mutex    mtx_;
};

} // namespace lemondory::db
