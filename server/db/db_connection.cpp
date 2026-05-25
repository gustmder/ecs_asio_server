#include "server/db/db_connection.hpp"
#include "common/log.hpp"

#ifdef LEMONDORY_HAVE_MYSQL

namespace lemondory::db {

// ─── DbConnection ─────────────────────────────────────────────────────────────

DbConnection::DbConnection(DbConnectionPool* pool, int index)
    : pool_(pool), index_(index) {}

DbConnection::~DbConnection() {
    if (pool_ && index_ >= 0) pool_->release(index_);
}

DbConnection::DbConnection(DbConnection&& o) noexcept
    : pool_(o.pool_), index_(o.index_) {
    o.pool_ = nullptr;
    o.index_ = -1;
}

DbConnection& DbConnection::operator=(DbConnection&& o) noexcept {
    if (this != &o) {
        if (pool_ && index_ >= 0) pool_->release(index_);
        pool_  = o.pool_;  o.pool_  = nullptr;
        index_ = o.index_; o.index_ = -1;
    }
    return *this;
}

MYSQL* DbConnection::get() const {
    return (pool_ && index_ >= 0) ? pool_->raw(index_) : nullptr;
}

// ─── DbConnectionPool ─────────────────────────────────────────────────────────

DbConnectionPool::DbConnectionPool(const lemondory::core::DbConfig& cfg)
    : cfg_(cfg) {}

DbConnectionPool::~DbConnectionPool() { close(); }

bool DbConnectionPool::open() {
    if (open_) return true;

    connections_.resize(static_cast<std::size_t>(cfg_.pool_size), nullptr);
    for (int i = 0; i < cfg_.pool_size; ++i) {
        MYSQL* c = mysql_init(nullptr);
        if (!c) {
            LOGE("mysql_init failed (index={})", i);
            close();
            return false;
        }

        bool reconnect = true;
        mysql_options(c, MYSQL_OPT_RECONNECT, &reconnect);

        unsigned int connect_timeout = 10;
        unsigned int rw_timeout      = 5;   // 쿼리 실행 타임아웃
        mysql_options(c, MYSQL_OPT_CONNECT_TIMEOUT, &connect_timeout);
        mysql_options(c, MYSQL_OPT_READ_TIMEOUT,    &rw_timeout);
        mysql_options(c, MYSQL_OPT_WRITE_TIMEOUT,   &rw_timeout);

        if (!mysql_real_connect(c,
                cfg_.host.c_str(),
                cfg_.user.c_str(),
                cfg_.password.c_str(),
                cfg_.name.c_str(),
                static_cast<unsigned int>(cfg_.port),
                nullptr, CLIENT_FOUND_ROWS)) {
            LOGE("mysql_real_connect failed: {}", mysql_error(c));
            mysql_close(c);
            close();
            return false;
        }

        connections_[static_cast<std::size_t>(i)] = c;
        free_indices_.push(i);
    }

    open_ = true;
    LOGI("DB pool opened ({} connections → {}:{})",
         cfg_.pool_size, cfg_.host, cfg_.port);
    return true;
}

void DbConnectionPool::close() {
    std::lock_guard<std::mutex> lk(mtx_);
    for (auto* c : connections_) {
        if (c) mysql_close(c);
    }
    connections_.clear();
    while (!free_indices_.empty()) free_indices_.pop();
    open_ = false;
}

DbConnection DbConnectionPool::acquire() {
    std::unique_lock<std::mutex> lk(mtx_);
    if (!cv_.wait_for(lk, std::chrono::seconds(5),
                      [this] { return !free_indices_.empty(); })) {
        LOGW("DB acquire timed out — pool exhausted?");
        return {};
    }
    int idx = free_indices_.front();
    free_indices_.pop();
    return DbConnection(this, idx);
}

void DbConnectionPool::release(int index) {
    {
        std::lock_guard<std::mutex> lk(mtx_);
        free_indices_.push(index);
    }
    cv_.notify_one();
}

MYSQL* DbConnectionPool::raw(int index) const {
    return connections_[static_cast<std::size_t>(index)];
}

} // namespace lemondory::db

#else // !LEMONDORY_HAVE_MYSQL

namespace lemondory::db {

DbConnection::DbConnection(DbConnectionPool* pool, int index)
    : pool_(pool), index_(index) {}
DbConnection::~DbConnection() {
    if (pool_ && index_ >= 0) pool_->release(index_);
}
DbConnection::DbConnection(DbConnection&& o) noexcept
    : pool_(o.pool_), index_(o.index_) { o.pool_ = nullptr; o.index_ = -1; }
DbConnection& DbConnection::operator=(DbConnection&& o) noexcept {
    if (this != &o) {
        if (pool_ && index_ >= 0) pool_->release(index_);
        pool_ = o.pool_; index_ = o.index_;
        o.pool_ = nullptr; o.index_ = -1;
    }
    return *this;
}

DbConnectionPool::DbConnectionPool(const lemondory::core::DbConfig& cfg) : cfg_(cfg) {}
DbConnectionPool::~DbConnectionPool() {}
bool DbConnectionPool::open() { return false; }
void DbConnectionPool::close() {}
DbConnection DbConnectionPool::acquire() { return {}; }
void DbConnectionPool::release(int) {}

} // namespace lemondory::db

#endif
