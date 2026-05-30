#pragma once
#include <string>
#include <optional>
#include <mutex>
#include <cstdint>

struct redisContext;

namespace lemondory::db {

class RedisClient {
public:
    struct Config {
        std::string host       = "127.0.0.1";
        int         port       = 6379;
        std::string password   = "";
        int         db_index   = 0;
        int         timeout_ms = 1000;
    };

    explicit RedisClient(const Config& cfg);
    ~RedisClient();

    RedisClient(const RedisClient&)            = delete;
    RedisClient& operator=(const RedisClient&) = delete;

    bool connect();
    void disconnect();
    bool is_connected() const;

    // token:{token} = player_id  (EX ttl_sec)
    bool set_token(const std::string& token, std::int64_t player_id, int ttl_sec = 3600);
    std::optional<std::int64_t> get_player_id_by_token(const std::string& token);
    bool del_token(const std::string& token);

    // player:{id}:data = json  (EX ttl_sec)
    bool set_player_cache(std::int64_t player_id, const std::string& json_data, int ttl_sec = 300);
    std::optional<std::string> get_player_cache(std::int64_t player_id);
    bool del_player_cache(std::int64_t player_id);

private:
    bool exec_set(const std::string& key, const std::string& value, int ttl_sec);
    std::optional<std::string> exec_get(const std::string& key);
    bool exec_del(const std::string& key);
    bool reconnect_if_needed();

    Config             cfg_;
    redisContext*      ctx_ = nullptr;
    mutable std::mutex mtx_;
};

} // namespace lemondory::db
