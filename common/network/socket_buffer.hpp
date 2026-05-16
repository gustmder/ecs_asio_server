#pragma once

#include <cstdint>
#include <mutex>

namespace lemondory::network {

class socket_buffer {
public:
    socket_buffer();
    ~socket_buffer();

    void init(std::int32_t buf_size, std::int32_t inc_count, bool use_lock);
    void free();

    bool put(char* data, std::int32_t size);
    bool pop(char* out, std::int32_t size, bool virtual_only = false);

private:
    bool inc();
    void lock();
    void unlock();

private:
    char* buffer_ = nullptr;
    std::int32_t max_ = 0;
    std::int32_t cur_ = 0;
    std::int32_t end_ = 0;
    std::int32_t use_ = 0;
    std::int32_t inc_ = 0;

    std::mutex mutex_;
    bool use_lock_ = false;
};

} // namespace lemondory::network