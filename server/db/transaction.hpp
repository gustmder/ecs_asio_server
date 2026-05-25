#pragma once
// MySQL이 없을 때는 이 헤더 전체가 no-op.
#ifdef LEMONDORY_HAVE_MYSQL
#include <mysql/mysql.h>
#include "server/db/db_connection.hpp"

namespace lemondory::db {

// RAII 트랜잭션: 생성 시 pool에서 커넥션 acquire + BEGIN.
// commit() 호출 시 COMMIT. 소멸 시 commit 안 됐으면 자동 ROLLBACK.
struct Transaction {
    DbConnection conn;
    bool committed_{false};

    explicit Transaction(DbConnectionPool& pool) : conn(pool.acquire()) {
        if (conn.valid()) mysql_query(conn.get(), "BEGIN");
    }

    bool commit() {
        if (!conn.valid()) return false;
        if (mysql_query(conn.get(), "COMMIT") != 0) return false;
        committed_ = true;
        return true;
    }

    ~Transaction() {
        if (conn.valid() && !committed_)
            mysql_query(conn.get(), "ROLLBACK");
    }

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    MYSQL* get() const { return conn.get(); }
    bool   valid() const { return conn.valid(); }
};

} // namespace lemondory::db
#endif // LEMONDORY_HAVE_MYSQL
