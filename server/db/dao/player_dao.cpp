#include "server/db/dao/player_dao.hpp"
#include "server/db/db_connection.hpp"
#include "common/log.hpp"
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>

#ifdef LEMONDORY_HAVE_MYSQL
#include <mysql/mysql.h>

namespace lemondory::db {

namespace {

struct StmtGuard {
    MYSQL_STMT* s;
    explicit StmtGuard(MYSQL_STMT* stmt) : s(stmt) {}
    ~StmtGuard() { if (s) mysql_stmt_close(s); }
    StmtGuard(const StmtGuard&) = delete;
};

// SELECT 로직 공유 헬퍼
std::optional<PlayerRecord> fetch_by_conn(MYSQL* mysql, const std::string& name) {
    const char* query =
        "SELECT id, name, level, exp, hp, max_hp, pos_x, pos_y, pos_z, map_id "
        "FROM players WHERE name = ?";

    StmtGuard sg{mysql_stmt_init(mysql)};
    if (!sg.s) return std::nullopt;
    if (mysql_stmt_prepare(sg.s, query, static_cast<unsigned long>(strlen(query)))) {
        LOGW("fetch_by_conn prepare: {}", mysql_stmt_error(sg.s));
        return std::nullopt;
    }

    unsigned long name_len   = static_cast<unsigned long>(name.size());
    MYSQL_BIND    param[1]   = {};
    param[0].buffer_type     = MYSQL_TYPE_STRING;
    param[0].buffer          = const_cast<char*>(name.c_str());
    param[0].buffer_length   = name_len;
    param[0].length          = &name_len;
    if (mysql_stmt_bind_param(sg.s, param)) return std::nullopt;
    if (mysql_stmt_execute(sg.s)) {
        LOGW("fetch_by_conn execute: {}", mysql_stmt_error(sg.s));
        return std::nullopt;
    }

    long long     id_val       = 0;
    char          name_buf[64] = {};
    unsigned long out_name_len = 0;
    int           level = 0, exp = 0, hp = 0, max_hp = 0, map_id = 0;
    float         pos_x = 0.f, pos_y = 0.f, pos_z = 0.f;

    MYSQL_BIND res[10]   = {};
    res[0].buffer_type   = MYSQL_TYPE_LONGLONG; res[0].buffer = &id_val;
    res[1].buffer_type   = MYSQL_TYPE_STRING;   res[1].buffer = name_buf;
    res[1].buffer_length = sizeof(name_buf);    res[1].length = &out_name_len;
    res[2].buffer_type   = MYSQL_TYPE_LONG;     res[2].buffer = &level;
    res[3].buffer_type   = MYSQL_TYPE_LONG;     res[3].buffer = &exp;
    res[4].buffer_type   = MYSQL_TYPE_LONG;     res[4].buffer = &hp;
    res[5].buffer_type   = MYSQL_TYPE_LONG;     res[5].buffer = &max_hp;
    res[6].buffer_type   = MYSQL_TYPE_FLOAT;    res[6].buffer = &pos_x;
    res[7].buffer_type   = MYSQL_TYPE_FLOAT;    res[7].buffer = &pos_y;
    res[8].buffer_type   = MYSQL_TYPE_FLOAT;    res[8].buffer = &pos_z;
    res[9].buffer_type   = MYSQL_TYPE_LONG;     res[9].buffer = &map_id;

    if (mysql_stmt_bind_result(sg.s, res)) return std::nullopt;
    int rc = mysql_stmt_fetch(sg.s);
    if (rc == MYSQL_NO_DATA) return std::nullopt;
    if (rc != 0) return std::nullopt;

    PlayerRecord rec;
    rec.id     = static_cast<std::int64_t>(id_val);
    rec.name   = std::string(name_buf, out_name_len);
    rec.level  = level; rec.exp    = exp;
    rec.hp     = hp;    rec.max_hp = max_hp;
    rec.pos_x  = pos_x; rec.pos_y  = pos_y; rec.pos_z = pos_z;
    rec.map_id = map_id;
    return rec;
}

// 재시도 가능 여부 + 백오프 딜레이 적용
void retry_delay(int attempt) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50 * attempt));
}

constexpr int kMaxAttempts = 3;

} // anonymous namespace

PlayerDao::PlayerDao(DbConnectionPool& pool) : pool_(pool) {}

std::optional<PlayerRecord> PlayerDao::find_by_name(const std::string& name) {
    auto conn = pool_.acquire();
    if (!conn.valid()) { LOGW("find_by_name: no DB connection"); return std::nullopt; }
    return fetch_by_conn(conn.get(), name);
}

std::optional<PlayerRecord> PlayerDao::find_or_create(const std::string& name) {
    const char* ins = "INSERT IGNORE INTO players (name) VALUES (?)";

    for (int attempt = 1; attempt <= kMaxAttempts; ++attempt) {
        auto conn = pool_.acquire();
        if (!conn.valid()) break;
        MYSQL* mysql = conn.get();

        // 1) INSERT IGNORE
        bool ins_ok = false;
        {
            StmtGuard sg{mysql_stmt_init(mysql)};
            if (sg.s && mysql_stmt_prepare(sg.s, ins, static_cast<unsigned long>(strlen(ins))) == 0) {
                unsigned long len = static_cast<unsigned long>(name.size());
                MYSQL_BIND p[1]   = {};
                p[0].buffer_type  = MYSQL_TYPE_STRING;
                p[0].buffer       = const_cast<char*>(name.c_str());
                p[0].buffer_length = len;
                p[0].length        = &len;
                mysql_stmt_bind_param(sg.s, p);
                if (mysql_stmt_execute(sg.s) == 0) {
                    ins_ok = true;
                } else {
                    unsigned int err = mysql_stmt_errno(sg.s);
                    LOGW("find_or_create INSERT attempt {}/{}: {} (errno={})",
                         attempt, kMaxAttempts, mysql_stmt_error(sg.s), err);
                    if (!is_retryable_mysql_error(err)) return std::nullopt;
                    if (attempt < kMaxAttempts) retry_delay(attempt);
                    continue;
                }
            }
        }

        // 2) SELECT
        auto rec = fetch_by_conn(mysql, name);
        if (rec) return rec;

        // SELECT 실패는 일반적으로 retryable
        LOGW("find_or_create SELECT attempt {}/{} returned nothing", attempt, kMaxAttempts);
        if (attempt < kMaxAttempts) retry_delay(attempt);
    }

    LOGE("find_or_create failed after {} attempts: name='{}'", kMaxAttempts, name);
    return std::nullopt;
}

bool PlayerDao::save_position(std::int64_t id, float x, float y, float z, int map_id) {
    char q[256];
    std::snprintf(q, sizeof(q),
        "UPDATE players SET pos_x=%.4f, pos_y=%.4f, pos_z=%.4f, map_id=%d, last_login=NOW() "
        "WHERE id=%lld",
        static_cast<double>(x), static_cast<double>(y), static_cast<double>(z),
        map_id, static_cast<long long>(id));

    for (int attempt = 1; attempt <= kMaxAttempts; ++attempt) {
        auto conn = pool_.acquire();
        if (!conn.valid()) break;

        if (mysql_query(conn.get(), q) == 0) return true;

        unsigned int err = mysql_errno(conn.get());
        LOGW("save_position attempt {}/{}: {} (id={} errno={})",
             attempt, kMaxAttempts, mysql_error(conn.get()), id, err);
        if (!is_retryable_mysql_error(err)) break;
        if (attempt < kMaxAttempts) retry_delay(attempt);
    }

    LOGE("save_position failed after {} attempts: id={}", kMaxAttempts, id);
    return false;
}

bool PlayerDao::save_stats(std::int64_t id, int level, int exp, int hp) {
    char q[256];
    std::snprintf(q, sizeof(q),
        "UPDATE players SET level=%d, exp=%d, hp=%d WHERE id=%lld",
        level, exp, hp, static_cast<long long>(id));

    for (int attempt = 1; attempt <= kMaxAttempts; ++attempt) {
        auto conn = pool_.acquire();
        if (!conn.valid()) break;

        if (mysql_query(conn.get(), q) == 0) return true;

        unsigned int err = mysql_errno(conn.get());
        LOGW("save_stats attempt {}/{}: {} (id={} errno={})",
             attempt, kMaxAttempts, mysql_error(conn.get()), id, err);
        if (!is_retryable_mysql_error(err)) break;
        if (attempt < kMaxAttempts) retry_delay(attempt);
    }

    LOGE("save_stats failed after {} attempts: id={}", kMaxAttempts, id);
    return false;
}

} // namespace lemondory::db

#else // !LEMONDORY_HAVE_MYSQL

namespace lemondory::db {
PlayerDao::PlayerDao(DbConnectionPool& pool) : pool_(pool) {}
std::optional<PlayerRecord> PlayerDao::find_by_name(const std::string&)         { return std::nullopt; }
std::optional<PlayerRecord> PlayerDao::find_or_create(const std::string&)       { return std::nullopt; }
bool PlayerDao::save_position(std::int64_t, float, float, float, int)           { return false; }
bool PlayerDao::save_stats(std::int64_t, int, int, int)                         { return false; }
} // namespace lemondory::db

#endif
