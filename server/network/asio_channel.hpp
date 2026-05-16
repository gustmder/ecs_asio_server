#pragma once

#include <asio.hpp>
#include <queue>
#include <memory>
#include <atomic>
#include <chrono>
#include "common/network/frame_codec.hpp"
#include "common/core/packet_buffer.hpp"
#include "common/core/spin_lock.hpp"
#include "common/network/socket_channel_base.hpp"
namespace lemondory::network
{
    class asio_server;

    class asio_channel : public socket_channel_base, public std::enable_shared_from_this<asio_channel> 
    {
    public:
        explicit asio_channel(asio::ip::tcp::socket socket, net_handler* net_handler);

        void send(const char* data, int size) override;
        void recv(char* buffer, int size) override;
        bool close(const void* error, close_function reason) override;

        void on_connect(net_error error) override;
        void on_accept() override;
        void on_close(const void* error, close_function reason) override;

        void start();

        using on_frame_callback = std::function<void(int, std::uint16_t, const char*, std::size_t)>;
        void set_on_frame(on_frame_callback cb) { on_frame_ = std::move(cb); }

    private:
        bool enqueue_send_(std::vector<char> buffer);
        void do_read();
        void do_write();

        // 하트비트/아이들 타이머
        void arm_idle_timer();
        void arm_ping_timer();
        void arm_stats_timer();
        void send_ping();
        static std::uint64_t now_ms() 
        {
            return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
        }
        void parse_frames_();

        asio::ip::tcp::socket socket_;

        std::size_t max_queue_bytes = 256 * 1024; // 최대 큐 크기 (256KB)
        std::atomic<std::size_t> current_queue_bytes{0}; // 현재 큐 크기

        std::atomic<bool> closed_{false};
        std::atomic<bool> writing_{false};

        std::atomic<std::uint64_t> enq_msgs{0}; // 전송된 메시지 수
        std::atomic<std::uint64_t> deq_msgs{0}; // 수신된 메시지 수
        std::atomic<std::uint64_t> drops{0}; // 드롭된 메시지 수
        std::atomic<std::uint64_t> force_closes{0}; // 강제 종료 수
        std::atomic<std::uint64_t> bytes_sent{0}; // 전송된 바이트 수
        std::atomic<std::uint64_t> peak_queued{0}; // 최대 큐 크기 기록

        std::atomic<std::uint64_t> last_recv_ms{0};
        std::atomic<std::uint64_t> last_send_ms{0};

        std::vector<char> read_buffer_;
        std::queue<std::vector<char>> write_queue_;
        core::spin_lock write_lock_;

        bool make_batch_(std::vector<char>& out, std::size_t& out_msgs);
        
        // 프레임 누적 버퍼 + 읽기 오프셋 (erase 없이 O(1) 소비)
        std::vector<char> frame_acc_;
        std::size_t frame_acc_offset_{0};
        
        // 하트 비트 추가 멤버
        asio::steady_timer idle_timer_{ socket_.get_executor() };
        asio::steady_timer ping_timer_{ socket_.get_executor() };
        asio::steady_timer stats_timer_{ socket_.get_executor() };

        std::chrono::seconds idle_limit_{ 30 };  // 수신 없으면 30초 후 timeout
        std::chrono::seconds ping_interval_{ 10 }; // 10초마다 PING 송신
        std::chrono::seconds stats_interval_{5};  // 통계 출력 주기 (기본 5초)

        // 프레임 최대 허용 크기 (안전 장치)
        // [EXTENSION POINT] 요 값은 운영 중 튜닝 가능
        static constexpr std::size_t max_frame_bytes_ = (1u << 20); // 1MB

        on_frame_callback on_frame_;
    };

} // namespace lemondory::network