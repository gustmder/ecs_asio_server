// packet_buffer.cpp
#include "packet_buffer.hpp"
#include "data_option.hpp"
#include <string>
#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace lemondory::core {

packet_buffer::packet_buffer() = default;

packet_buffer::packet_buffer(std::size_t initial_capacity, data_option option) 
    : is_dynamic_(true), is_owner_(true), option_(option) {
    dynamic_buffer_.reserve(initial_capacity);
    size_ = static_cast<std::int32_t>(initial_capacity);
}

packet_buffer::packet_buffer(char* buffer, std::int32_t offset, std::int32_t size, data_option option) {
    set(buffer, offset, size, option);
}

packet_buffer::packet_buffer(const packet_buffer& other) 
    : dynamic_buffer_(other.dynamic_buffer_),
      external_buffer_(other.external_buffer_),
      size_(other.size_),
      offset_(other.offset_),
      option_(other.option_),
      is_dynamic_(other.is_dynamic_),
      is_owner_(other.is_owner_) {
}

packet_buffer::packet_buffer(packet_buffer&& other) noexcept
    : dynamic_buffer_(std::move(other.dynamic_buffer_)),
      external_buffer_(other.external_buffer_),
      size_(other.size_),
      offset_(other.offset_),
      option_(other.option_),
      is_dynamic_(other.is_dynamic_),
      is_owner_(other.is_owner_) {
    other.external_buffer_ = nullptr;
    other.size_ = 0;
    other.offset_ = 0;
    other.is_dynamic_ = false;
    other.is_owner_ = false;
}

packet_buffer& packet_buffer::operator=(const packet_buffer& other) {
    if (this != &other) {
        dynamic_buffer_ = other.dynamic_buffer_;
        external_buffer_ = other.external_buffer_;
        size_ = other.size_;
        offset_ = other.offset_;
        option_ = other.option_;
        is_dynamic_ = other.is_dynamic_;
        is_owner_ = other.is_owner_;
    }
    return *this;
}

packet_buffer& packet_buffer::operator=(packet_buffer&& other) noexcept {
    if (this != &other) {
        dynamic_buffer_ = std::move(other.dynamic_buffer_);
        external_buffer_ = other.external_buffer_;
        size_ = other.size_;
        offset_ = other.offset_;
        option_ = other.option_;
        is_dynamic_ = other.is_dynamic_;
        is_owner_ = other.is_owner_;
        
        other.external_buffer_ = nullptr;
        other.size_ = 0;
        other.offset_ = 0;
        other.is_dynamic_ = false;
        other.is_owner_ = false;
    }
    return *this;
}

void packet_buffer::reset() {
    dynamic_buffer_.clear();
    external_buffer_ = nullptr;
    size_ = 0;
    offset_ = 0;
    option_ = data_option::none;
    is_dynamic_ = false;
    is_owner_ = false;
}

void packet_buffer::set(char* buffer, std::int32_t offset, std::int32_t size, data_option option) {
    dynamic_buffer_.clear();
    external_buffer_ = buffer;
    size_ = size;
    offset_ = offset;
    option_ = option;
    is_dynamic_ = false;
    is_owner_ = false;
}

void packet_buffer::set_dynamic(std::size_t capacity, data_option option) {
    dynamic_buffer_.clear();
    dynamic_buffer_.reserve(capacity);
    external_buffer_ = nullptr;
    size_ = static_cast<std::int32_t>(capacity);
    offset_ = 0;
    option_ = option;
    is_dynamic_ = true;
    is_owner_ = true;
}

void packet_buffer::set_offset(std::int32_t offset) {
    if (offset >= 0 && offset <= size_) {
        offset_ = offset;
    }
}

std::int32_t packet_buffer::offset() const {
    return offset_;
}

std::int32_t packet_buffer::size() const {
    return size_;
}

std::int32_t packet_buffer::remaining_size() const {
    return size_ - offset_;
}

std::int32_t packet_buffer::capacity() const {
    if (is_dynamic_) {
        return static_cast<std::int32_t>(dynamic_buffer_.capacity());
    }
    return size_;
}

char* packet_buffer::raw() {
    return get_buffer_ptr();
}

const char* packet_buffer::raw() const {
    return get_buffer_ptr();
}

char* packet_buffer::raw_offset() {
    return get_buffer_ptr() + offset_;
}

const char* packet_buffer::raw_offset() const {
    return get_buffer_ptr() + offset_;
}

data_option packet_buffer::option() const {
    return option_;
}

void packet_buffer::append(const char* data, std::int32_t length) {
    if (length <= 0) return;
    
    ensure_capacity(offset_ + length);
    
    char* buffer = get_buffer_ptr();
    std::memcpy(buffer + offset_, data, length);
    offset_ += length;
    
    if (is_dynamic_) {
        dynamic_buffer_.resize(std::max(static_cast<std::size_t>(offset_), dynamic_buffer_.size()));
    }
}

void packet_buffer::append(const std::string& data) {
    append(data.data(), static_cast<std::int32_t>(data.size()));
}

void packet_buffer::append(const std::vector<char>& data) {
    append(data.data(), static_cast<std::int32_t>(data.size()));
}

void packet_buffer::reserve(std::int32_t capacity) {
    if (is_dynamic_) {
        dynamic_buffer_.reserve(capacity);
    }
}

void packet_buffer::resize(std::int32_t size) {
    if (size < 0) return;
    
    if (is_dynamic_) {
        dynamic_buffer_.resize(size);
        this->size_ = size;
        if (offset_ > size) {
            offset_ = size;
        }
    } else {
        if (size <= size_) {
            this->size_ = size;
            if (offset_ > size) {
                offset_ = size;
            }
        }
    }
}

char* packet_buffer::get_buffer_ptr() {
    return is_dynamic_ ? dynamic_buffer_.data() : external_buffer_;
}

const char* packet_buffer::get_buffer_ptr() const {
    return is_dynamic_ ? dynamic_buffer_.data() : external_buffer_;
}

void packet_buffer::ensure_capacity(std::int32_t required_size) {
    if (is_dynamic_) {
        if (static_cast<std::size_t>(required_size) > dynamic_buffer_.capacity()) {
            dynamic_buffer_.reserve(required_size);
        }
    } else {
        if (required_size > size_) {
            // 외부 버퍼는 확장할 수 없음
            throw std::runtime_error("External buffer capacity exceeded");
        }
    }
}

} // namespace lemondory::core
