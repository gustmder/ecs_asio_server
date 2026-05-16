// socket_buffer.cpp

#include "socket_buffer.hpp"
#include <cstring>
#include <cstdlib>

namespace lemondory::network {

socket_buffer::socket_buffer() {
    free();
}

socket_buffer::~socket_buffer() {
    std::free(buffer_);
    buffer_ = nullptr;
}

void socket_buffer::init(std::int32_t buf_size, std::int32_t inc_count, bool use_lock) {
    use_lock_ = use_lock;
    lock();
    buffer_ = static_cast<char*>(std::malloc(sizeof(char) * buf_size));
    max_ = buf_size;
    inc_ = inc_count;
    unlock();
}

void socket_buffer::free() {
    lock();
    cur_ = 0;
    end_ = 0;
    use_ = 0;
    unlock();
}

bool socket_buffer::inc() {
    std::int32_t new_size = max_ * 2;
    char* tmp = static_cast<char*>(std::realloc(buffer_, sizeof(char) * new_size));
    if (!tmp) return false;
    buffer_ = tmp;

    if (cur_ > end_) {
        std::memcpy(buffer_ + max_, buffer_, end_);
        end_ = cur_ + use_;
    }

    max_ = new_size;
    return true;
}

bool socket_buffer::put(char* data, std::int32_t size) {
    lock();
    if (!data || size < 0) {
        unlock();
        return false;
    }

    if (size >= max_ - use_) {
        bool success = false;
        for (int i = 0; i < inc_; ++i) {
            if (inc()) {
                if (size <= max_ - use_) {
                    success = true;
                    break;
                }
            }
        }
        if (!success) {
            unlock();
            return false;
        }
    }

    if (size <= max_ - end_) {
        std::memcpy(buffer_ + end_, data, size);
        end_ += size;
    } else {
        std::int32_t first = max_ - end_;
        std::int32_t second = size - first;
        std::memcpy(buffer_ + end_, data, first);
        std::memcpy(buffer_, data + first, second);
        end_ = second;
    }

    use_ += size;
    unlock();
    return true;
}

bool socket_buffer::pop(char* out, std::int32_t size, bool virtual_only) {
    lock();
    if (size < 0 || size > use_) {
        unlock();
        return false;
    }

    if (!virtual_only && out) {
        if (size <= max_ - cur_) {
            std::memcpy(out, buffer_ + cur_, size);
        } else {
            std::int32_t first = max_ - cur_;
            std::int32_t second = size - first;
            std::memcpy(out, buffer_ + cur_, first);
            std::memcpy(out + first, buffer_, second);
        }
    }

    cur_ = (cur_ + size) % max_;
    use_ -= size;
    unlock();
    return true;
}

void socket_buffer::lock() {
    if (use_lock_) mutex_.lock();
}

void socket_buffer::unlock() {
    if (use_lock_) mutex_.unlock();
}

} // namespace lemondory::network
