#include "server/game/game_server.hpp"
#include "common/log.hpp"
#include "common/protocol/protocol_guild.hpp"
#include <algorithm>

namespace lemondory::game {
using namespace lemondory::network;

// ── 핸들러 등록 ──────────────────────────────────────────────────────────────

void game_server::bind_guild_handlers() {
    dispatcher_.bind(2001, [this](int ch, const char* d, std::size_t s) { handle_guild_create(ch, d, s); });
    dispatcher_.bind(2002, [this](int ch, const char* d, std::size_t s) { handle_guild_join(ch, d, s); });
    dispatcher_.bind(2003, [this](int ch, const char* d, std::size_t s) { handle_guild_leave(ch, d, s); });
}

// ── 2001 GUILD_CREATE ────────────────────────────────────────────────────────

void game_server::handle_guild_create(int channel_id, const char* data, std::size_t size) {
    using namespace lemondory::shared;

    guild_create_req req;
    if (!guild_create_req::parse(data, data + size, req)) {
        LOGW("Invalid guild_create_req from channel={}", channel_id);
        return;
    }

    std::lock_guard<std::mutex> lk(guild_mutex_);

    if (channel_to_guild_.count(channel_id)) {
        guild_create_res res{false, "already in a guild", 0};
        auto payload = res.serialize();
        send_test_frame(channel_id, 2001, payload.data(), payload.size());
        return;
    }

    std::uint32_t guild_id = next_guild_id_++;
    GuildData guild{guild_id, req.guild_name, req.guild_description, channel_id, {channel_id}};
    guilds_[guild_id] = std::move(guild);
    channel_to_guild_[channel_id] = guild_id;

    LOGI("Guild created: id={} name='{}' by channel={}", guild_id, req.guild_name, channel_id);

    guild_create_res res{true, "ok", guild_id};
    auto payload = res.serialize();
    send_test_frame(channel_id, 2001, payload.data(), payload.size());
}

// ── 2002 GUILD_JOIN ──────────────────────────────────────────────────────────

void game_server::handle_guild_join(int channel_id, const char* data, std::size_t size) {
    using namespace lemondory::shared;

    guild_join_req req;
    if (!guild_join_req::parse(data, data + size, req)) {
        LOGW("Invalid guild_join_req from channel={}", channel_id);
        return;
    }

    std::lock_guard<std::mutex> lk(guild_mutex_);

    auto git = guilds_.find(req.guild_id);
    if (git == guilds_.end()) {
        guild_join_res res{false, "guild not found", 0};
        auto payload = res.serialize();
        send_test_frame(channel_id, 2002, payload.data(), payload.size());
        return;
    }
    if (channel_to_guild_.count(channel_id)) {
        guild_join_res res{false, "already in a guild", 0};
        auto payload = res.serialize();
        send_test_frame(channel_id, 2002, payload.data(), payload.size());
        return;
    }

    git->second.members.push_back(channel_id);
    channel_to_guild_[channel_id] = req.guild_id;

    LOGI("channel={} joined guild={} ({})", channel_id, req.guild_id, git->second.name);

    guild_join_res res{true, "ok", 0};
    auto payload = res.serialize();
    send_test_frame(channel_id, 2002, payload.data(), payload.size());
}

// ── 2003 GUILD_LEAVE ─────────────────────────────────────────────────────────

void game_server::handle_guild_leave(int channel_id, const char* data, std::size_t size) {
    using namespace lemondory::shared;
    (void)data; (void)size;

    std::lock_guard<std::mutex> lk(guild_mutex_);

    auto cit = channel_to_guild_.find(channel_id);
    if (cit == channel_to_guild_.end()) {
        LOGW("channel={} not in any guild (leave)", channel_id);
        return;
    }

    std::uint32_t guild_id = cit->second;
    channel_to_guild_.erase(cit);

    auto git = guilds_.find(guild_id);
    if (git != guilds_.end()) {
        auto& members = git->second.members;
        members.erase(std::remove(members.begin(), members.end(), channel_id), members.end());
        if (members.empty()) {
            LOGI("Guild {} disbanded (no members)", guild_id);
            guilds_.erase(git);
        }
    }

    LOGI("channel={} left guild={}", channel_id, guild_id);

    guild_leave_res res{true, "ok"};
    auto payload = res.serialize();
    send_test_frame(channel_id, 2003, payload.data(), payload.size());
}

} // namespace lemondory::game
