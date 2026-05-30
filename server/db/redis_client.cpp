#include "redis_client.hpp"
#include "common/log.hpp"
#include <hiredis.h>
#include <stdexcept>
#include <cstring>

namespace lemondory::db {

RedisClient::RedisClient(const Config& cfg) : cfg_(cfg) {}

RedisClient::~RedisClient() {
    disconnect();
}

bool RedisClient::connect() {
    std::lock_guard<std::mutex> lk(mtx_);

    struct timeval tv { cfg_.timeout_ms / 1000, (cfg_.timeout_ms % 1000) * 1000 };
    ctx_ = redisConnectWithTimeout(cfg_.host.c_str(), cfg_.port, tv);

    if (!ctx_ || ctx_->err) {
        LOGE("Redis connect failed: {}:{} — {}",
             cfg_.host, cfg_.port, ctx_ ? ctx_->errstr : "null context");
        if (ctx_) { redisFree(ctx_); ctx_ = nullptr; }
        return false;
    }

    if (!cfg_.password.empty()) {
        redisReply* r = static_cast<redisReply*>(
            redisCommand(ctx_, "AUTH %s", cfg_.password.c_str()));
        bool ok = r && r->type != REDIS_REPLY_ERROR;
        if (r) freeReplyObject(r);
        if (!ok) {
            LOGE("Redis AUTH failed");
            redisFree(ctx_); ctx_ = nullptr;
            return false;
        }
    }

    if (cfg_.db_index != 0) {
        redisReply* r = static_cast<redisReply*>(
            redisCommand(ctx_, "SELECT %d", cfg_.db_index));
        bool ok = r && r->type != REDIS_REPLY_ERROR;
        if (r) freeReplyObject(r);
        if (!ok) {
            LOGE("Redis SELECT {} failed", cfg_.db_index);
            redisFree(ctx_); ctx_ = nullptr;
            return false;
        }
    }

    LOGI("Redis connected: {}:{} db={}", cfg_.host, cfg_.port, cfg_.db_index);
    return true;
}

void RedisClient::disconnect() {
    std::lock_guard<std::mutex> lk(mtx_);
    if (ctx_) {
        redisFree(ctx_);
        ctx_ = nullptr;
        LOGI("Redis disconnected");
    }
}

bool RedisClient::is_connected() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return ctx_ != nullptr && ctx_->err == 0;
}

bool RedisClient::reconnect_if_needed() {
    if (ctx_ && ctx_->err == 0) return true;
    if (ctx_) { redisFree(ctx_); ctx_ = nullptr; }

    struct timeval tv { cfg_.timeout_ms / 1000, (cfg_.timeout_ms % 1000) * 1000 };
    ctx_ = redisConnectWithTimeout(cfg_.host.c_str(), cfg_.port, tv);
    if (!ctx_ || ctx_->err) {
        if (ctx_) { redisFree(ctx_); ctx_ = nullptr; }
        return false;
    }
    if (!cfg_.password.empty()) {
        redisReply* r = static_cast<redisReply*>(
            redisCommand(ctx_, "AUTH %s", cfg_.password.c_str()));
        bool ok = r && r->type != REDIS_REPLY_ERROR;
        if (r) freeReplyObject(r);
        if (!ok) { redisFree(ctx_); ctx_ = nullptr; return false; }
    }
    if (cfg_.db_index != 0) {
        redisReply* r = static_cast<redisReply*>(
            redisCommand(ctx_, "SELECT %d", cfg_.db_index));
        bool ok = r && r->type != REDIS_REPLY_ERROR;
        if (r) freeReplyObject(r);
        if (!ok) { redisFree(ctx_); ctx_ = nullptr; return false; }
    }
    LOGD("Redis reconnected");
    return true;
}

bool RedisClient::exec_set(const std::string& key, const std::string& value, int ttl_sec) {
    if (!reconnect_if_needed()) return false;
    redisReply* r = static_cast<redisReply*>(
        redisCommand(ctx_, "SET %s %s EX %d", key.c_str(), value.c_str(), ttl_sec));
    bool ok = r && r->type != REDIS_REPLY_ERROR;
    if (r) freeReplyObject(r);
    return ok;
}

std::optional<std::string> RedisClient::exec_get(const std::string& key) {
    if (!reconnect_if_needed()) return std::nullopt;
    redisReply* r = static_cast<redisReply*>(redisCommand(ctx_, "GET %s", key.c_str()));
    if (!r) return std::nullopt;
    std::optional<std::string> result;
    if (r->type == REDIS_REPLY_STRING) result = std::string(r->str, r->len);
    freeReplyObject(r);
    return result;
}

bool RedisClient::exec_del(const std::string& key) {
    if (!reconnect_if_needed()) return false;
    redisReply* r = static_cast<redisReply*>(redisCommand(ctx_, "DEL %s", key.c_str()));
    bool ok = r != nullptr;
    if (r) freeReplyObject(r);
    return ok;
}

// ── 토큰 ────────────────────────────────────────────────────────────────────

bool RedisClient::set_token(const std::string& token, std::int64_t player_id, int ttl_sec) {
    std::lock_guard<std::mutex> lk(mtx_);
    return exec_set("token:" + token, std::to_string(player_id), ttl_sec);
}

std::optional<std::int64_t> RedisClient::get_player_id_by_token(const std::string& token) {
    std::lock_guard<std::mutex> lk(mtx_);
    auto val = exec_get("token:" + token);
    if (!val) return std::nullopt;
    try { return static_cast<std::int64_t>(std::stoll(*val)); }
    catch (...) { return std::nullopt; }
}

bool RedisClient::del_token(const std::string& token) {
    std::lock_guard<std::mutex> lk(mtx_);
    return exec_del("token:" + token);
}

// ── 플레이어 캐시 ───────────────────────────────────────────────────────────

bool RedisClient::set_player_cache(std::int64_t player_id, const std::string& json_data, int ttl_sec) {
    std::lock_guard<std::mutex> lk(mtx_);
    return exec_set("player:" + std::to_string(player_id) + ":data", json_data, ttl_sec);
}

std::optional<std::string> RedisClient::get_player_cache(std::int64_t player_id) {
    std::lock_guard<std::mutex> lk(mtx_);
    return exec_get("player:" + std::to_string(player_id) + ":data");
}

bool RedisClient::del_player_cache(std::int64_t player_id) {
    std::lock_guard<std::mutex> lk(mtx_);
    return exec_del("player:" + std::to_string(player_id) + ":data");
}

} // namespace lemondory::db
