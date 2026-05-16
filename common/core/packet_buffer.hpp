// packet_buffer.hpp
#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "data_option.hpp"

namespace lemondory::core {

// 패킷 데이터를 임시 저장 및 관리하기 위한 버퍼 클래스
class packet_buffer {
public:
    packet_buffer();
    
    // 동적 크기 버퍼로 초기화
    packet_buffer(std::size_t initial_capacity, data_option option = data_option::none);
    
    // 외부 버퍼로 초기화 (소유권 없음)
    packet_buffer(char* buffer, std::int32_t offset, std::int32_t size, data_option option);

    // 복사 생성자
    packet_buffer(const packet_buffer& other);
    
    // 이동 생성자
    packet_buffer(packet_buffer&& other) noexcept;
    
    // 대입 연산자
    packet_buffer& operator=(const packet_buffer& other);
    packet_buffer& operator=(packet_buffer&& other) noexcept;

    // 소멸자
    ~packet_buffer() = default;

    // 모든 설정 초기화
    void reset();

    // 버퍼 및 설정값 재설정
    void set(char* buffer, std::int32_t offset, std::int32_t size, data_option option);
    
    // 동적 버퍼로 재설정
    void set_dynamic(std::size_t capacity, data_option option = data_option::none);

    // 오프셋 설정 및 조회
    void set_offset(std::int32_t offset);
    [[nodiscard]] std::int32_t offset() const;
    [[nodiscard]] std::int32_t size() const;
    [[nodiscard]] std::int32_t remaining_size() const;
    [[nodiscard]] std::int32_t capacity() const;

    // 원시 버퍼 접근자
    [[nodiscard]] char* raw();
    [[nodiscard]] const char* raw() const;
    [[nodiscard]] char* raw_offset();
    [[nodiscard]] const char* raw_offset() const;

    // 전송 옵션 조회
    [[nodiscard]] data_option option() const;

    // 추가 데이터를 버퍼에 저장
    void append(const char* data, std::int32_t length);
    void append(const std::string& data);
    void append(const std::vector<char>& data);
    
    // 버퍼 확장
    void reserve(std::int32_t capacity);
    void resize(std::int32_t size);
    
    // 버퍼 상태 확인
    [[nodiscard]] bool is_dynamic() const { return is_dynamic_; }
    [[nodiscard]] bool is_owner() const { return is_owner_; }

private:
    // 동적 버퍼 (소유권 있음)
    std::vector<char> dynamic_buffer_;
    
    // 외부 버퍼 (소유권 없음)
    char* external_buffer_ = nullptr;
    
    std::int32_t size_ = 0;                    // 전체 버퍼 크기
    std::int32_t offset_ = 0;                  // 현재 읽기/쓰기 오프셋
    data_option option_ = data_option::none;   // 데이터 처리 옵션
    
    bool is_dynamic_ = false;                  // 동적 버퍼 사용 여부
    bool is_owner_ = false;                    // 버퍼 소유권 여부
    
    // 내부 헬퍼
    char* get_buffer_ptr();
    const char* get_buffer_ptr() const;
    void ensure_capacity(std::int32_t required_size);
};

} // namespace lemondory::core