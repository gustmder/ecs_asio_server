#pragma once
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include "server/db/db_connection.hpp"
#include "server/core/server_config.hpp"

namespace lemondory::db {

enum class DbRole : std::size_t {
    Game   = 0,
    Guild  = 1,
    Common = 2,
};

class DbManager {
public:
    // 활성화된 role 중 하나라도 연결되면 true. 실패한 role은 pool(role) == nullptr.
    bool init(const std::unordered_map<std::string, lemondory::core::DbConfig>& configs);
    void close();

    // role이 비활성이거나 연결 실패했으면 nullptr 반환.
    DbConnectionPool* pool(DbRole role) const;

private:
    static const char* role_key(DbRole role);

    static constexpr std::size_t kCount = 3;
    std::array<std::unique_ptr<DbConnectionPool>, kCount> pools_{};
};

} // namespace lemondory::db
