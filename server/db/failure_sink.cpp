#include "server/db/failure_sink.hpp"
#include "common/log.hpp"
#include <cstdio>
#include <ctime>
#include <chrono>
#include <filesystem>

namespace lemondory::db {

FailureSink::FailureSink(const std::string& path) {
    std::error_code ec;
    std::filesystem::create_directories(
        std::filesystem::path(path).parent_path(), ec);

    file_.open(path, std::ios::app);
    if (!file_.is_open())
        LOGW("FailureSink: 파일 열기 실패 — 스냅샷 기록 불가 ({})", path);
    else
        LOGI("FailureSink: 스냅샷 파일 준비됨 ({})", path);
}

FailureSink::~FailureSink() {
    if (file_.is_open()) file_.close();
}

void FailureSink::record(const PlayerSaveSnapshot& s) {
    if (!file_.is_open()) return;

    // UTC ISO 8601 타임스탬프
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));

    // 문자열 내 따옴표 이스케이프
    auto esc = [](const std::string& src) {
        std::string r;
        r.reserve(src.size());
        for (char c : src) {
            if (c == '"')  { r += "\\\""; }
            else if (c == '\\') { r += "\\\\"; }
            else           { r += c; }
        }
        return r;
    };

    // JSON Lines 포맷 — 한 줄 = 한 레코드
    char line[1024];
    std::snprintf(line, sizeof(line),
        R"({"ts":"%s","op":"%s","player_id":%lld,"name":"%s",)"
        R"("x":%.4f,"y":%.4f,"z":%.4f,"map_id":%d,)"
        R"("level":%d,"exp":%d,"hp":%d,"attempts":%d,"error":"%s"})",
        ts,
        s.operation.c_str(),
        static_cast<long long>(s.player_id),
        esc(s.player_name).c_str(),
        s.pos_x, s.pos_y, s.pos_z, s.map_id,
        s.level, s.exp, s.hp, s.attempts,
        esc(s.error_msg).c_str());

    {
        std::lock_guard<std::mutex> lk(mtx_);
        file_ << line << '\n';
        file_.flush();  // 서버 크래시 시에도 디스크에 반영되도록 즉시 flush
    }

    LOGE("DB 저장 실패 — 스냅샷 기록: player_id={} op={} error={}",
         s.player_id, s.operation, s.error_msg);
}

} // namespace lemondory::db
