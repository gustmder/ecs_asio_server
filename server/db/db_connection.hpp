#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include "server/core/server_config.hpp"

#ifdef LEMONDORY_HAVE_MYSQL
#include <mysql/mysql.h>

namespace lemondory::db {
// 연결 끊김·데드락·락 타임아웃은 재시도 가능. 권한 오류·중복키 등은 재시도 불필요.
// MySQL 에러 코드는 프로토콜 레벨 상수 — 버전과 무관하게 안정적.
inline bool is_retryable_mysql_error(unsigned int err) noexcept {
    return err == 2006   // CR_SERVER_GONE_ERROR: 연결 끊김
        || err == 2013   // CR_SERVER_LOST:       쿼리 중 연결 끊김
        || err == 1213   // ER_LOCK_DEADLOCK:     데드락
        || err == 1205;  // ER_LOCK_WAIT_TIMEOUT: 락 대기 타임아웃
}
} // namespace lemondory::db

#endif

namespace lemondory::db {

class DbConnectionPool;

class DbConnection {
public:
    DbConnection() = default;
    DbConnection(DbConnectionPool* pool, int index);
    ~DbConnection();

    DbConnection(const DbConnection&) = delete;
    DbConnection& operator=(const DbConnection&) = delete;
    DbConnection(DbConnection&&) noexcept;
    DbConnection& operator=(DbConnection&&) noexcept;

    bool valid() const { return pool_ != nullptr && index_ >= 0; }

#ifdef LEMONDORY_HAVE_MYSQL
    MYSQL* get() const;
#endif

private:
    DbConnectionPool* pool_{nullptr};
    int index_{-1};
};

class DbConnectionPool {
public:
    explicit DbConnectionPool(const lemondory::core::DbConfig& cfg);
    ~DbConnectionPool();

    bool open();
    void close();

    // Blocks up to 5 s; returns invalid DbConnection on timeout.
    DbConnection acquire();
    void release(int index);

#ifdef LEMONDORY_HAVE_MYSQL
    MYSQL* raw(int index) const;
#endif

private:
    lemondory::core::DbConfig cfg_;
#ifdef LEMONDORY_HAVE_MYSQL
    std::vector<MYSQL*> connections_;
#endif
    std::queue<int> free_indices_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool open_{false};
};

} // namespace lemondory::db
