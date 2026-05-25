#include "server/db/db_manager.hpp"
#include "common/log.hpp"

namespace lemondory::db {

const char* DbManager::role_key(DbRole role) {
    switch (role) {
    case DbRole::Game:   return "game";
    case DbRole::Guild:  return "guild";
    case DbRole::Common: return "common";
    default:             return "";
    }
}

bool DbManager::init(const std::unordered_map<std::string, lemondory::core::DbConfig>& configs) {
    int opened = 0;
    for (auto role : {DbRole::Game, DbRole::Guild, DbRole::Common}) {
        const char* key = role_key(role);
        auto it = configs.find(key);
        if (it == configs.end() || !it->second.enabled) continue;

        auto p = std::make_unique<DbConnectionPool>(it->second);
        if (p->open()) {
            pools_[static_cast<std::size_t>(role)] = std::move(p);
            LOGI("DbManager: pool '{}' opened", key);
            ++opened;
        } else {
            LOGW("DbManager: pool '{}' failed to open", key);
        }
    }
    return opened > 0;
}

void DbManager::close() {
    for (auto& p : pools_) {
        if (p) { p->close(); p.reset(); }
    }
}

DbConnectionPool* DbManager::pool(DbRole role) const {
    auto idx = static_cast<std::size_t>(role);
    return (idx < pools_.size()) ? pools_[idx].get() : nullptr;
}

} // namespace lemondory::db
