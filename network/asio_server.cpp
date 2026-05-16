// asio_server.cpp

#include "asio_server.hpp"
#include "asio_channel.hpp"
#include "socket_channel_base.hpp"
#include "socket_platform.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace lemondory::network {

asio_server::asio_server(asio::io_context& io_context)
    : io_context_(io_context), acceptor_(io_context) {}

asio_server::~asio_server() {
    stop();
}

bool asio_server::init(net_handler* handler, const socket_option& option, int init_channel_count, int io_thread_count) {
    handler_ = handler;
    socket_option_ = option;
    return true;  // 추가 초기화 작업이 필요한 경우 여기에 작성
}

bool asio_server::listen(const std::string& ip_address, int port) {
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip_address), port);
    asio::error_code ec;

    // acceptor가 이미 열려있다면 완전히 정리
    if (acceptor_.is_open()) {
        acceptor_.close(ec);
        // 잠시 대기하여 완전히 정리되도록 함
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 새로운 acceptor 생성 (더 안전한 방법)
    acceptor_ = asio::ip::tcp::acceptor(io_context_);
    
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "Failed to open acceptor: " << ec.message() << std::endl;
        return false;
    }

    acceptor_.set_option(asio::socket_base::reuse_address(socket_option_.reuse_address()), ec);
    if (ec) {
        std::cerr << "Failed to set socket option: " << ec.message() << std::endl;
        return false;
    }

    acceptor_.bind(endpoint, ec);
    if (ec) {
        std::cerr << "Failed to bind to " << ip_address << ":" << port << " - " << ec.message() << " (error code: " << ec.value() << ")" << std::endl;
        return false;
    }

    acceptor_.listen(socket_option_.backlog(), ec);
    if (ec) {
        std::cerr << "Failed to listen: " << ec.message() << std::endl;
        return false;
    }

    do_accept();
    return true;
}

bool asio_server::stop() {
    asio::error_code ec;
    acceptor_.close(ec);
    return !ec;
}

void asio_server::release() {
    stop();
    handler_ = nullptr;
}

void asio_server::set_net_handler(net_handler* handler) {
    handler_ = handler;
}

net_handler* asio_server::get_net_handler() {
    return handler_;
}

// void asio_server::do_accept() {
//     acceptor_.async_accept(
//         [this](asio::error_code ec, asio::ip::tcp::socket socket) {
//             if (!ec) {
//                 auto channel = std::make_shared<asio_channel>(std::move(socket), handler_);
//                 channel->start();
//             } else {
//                 std::cerr << "Accept error: " << ec.message() << std::endl;
//             }
//             do_accept();
//         });
// }

// void asio_server::do_accept() 
// {
//     acceptor_.async_accept([this, self = shared_from_this()](asio::error_code ec, asio::ip::tcp::socket socket) 
//     {
//         if (ec) {
//             std::cerr << "[server] accept error: (" << ec.value() << ") " << ec.message() << "\n";
//             do_accept();
//             return;
//         }

//         auto channel = std::make_shared<asio_channel>(std::move(socket), handler_);
//         // 새 채널이 만들어질 때, 채널 → 서버 콜백으로 프레임을 중계
//         channel->set_on_frame([this](int channel_id, std::uint16_t cmd, const char* payload, std::size_t size)
//         {
//             if (on_frame_) 
//             {
//                 on_frame_(channel_id, cmd, payload, size);
//             }
//         });


//         // 1) 채널 ID 발급
//         int id = next_channel_id_++;

//         // 2) 서버 맵에 등록 (수명/관리)
//         {
//             std::lock_guard<std::mutex> lk(channel_mutex_);
//             channels_[id] = channel;
//         }

//         // 3) 채널에 ID 주입
//         channel->set_channel_id(id);
//         std::cout << "[server] accepted channel id=" << id << "\n";

//         // 4) 수신 시작 (on_accept는 채널 안에서 호출)
//         channel->start();

//         // 다음 accept
//         do_accept();
//     });
// }

// asio_server.cpp
void asio_server::do_accept()
{
    acceptor_.async_accept(
        [this](asio::error_code ec, asio::ip::tcp::socket socket)
        {
            if (ec) {
                std::cerr << "[server] accept error: (" << ec.value() << ") " << ec.message() << "\n";
                // acceptor 가 정상 동작 중이면 계속 대기
                do_accept();
                return;
            }

            // (선택) 수락 직후 소켓 옵션 적용 — 필요 시 활성화
            // asio::error_code opt_ec;
            // socket.set_option(asio::ip::tcp::no_delay(socket_option_.tcp_no_delay()), opt_ec);
            // // reuse_address 는 리스너에 적용됨. linger 등은 정책에 따라 채널에서 처리.

            auto channel = std::make_shared<asio_channel>(std::move(socket), handler_);

            // 채널 → 서버 프레임 중계
            channel->set_on_frame([this](int channel_id, std::uint16_t cmd, const char* payload, std::size_t size)
            {
                if (on_frame_) {
                    on_frame_(channel_id, cmd, payload, size);
                }
            });

            // ID 발급 (원자적 연산)
            int id = next_channel_id_.fetch_add(1, std::memory_order_relaxed);
            
            // 채널 맵 등록 (뮤텍스 보호)
            {
                std::lock_guard<std::mutex> lk(channel_mutex_);
                channels_[id] = channel;
            }

            channel->set_channel_id(id);
            std::cout << "[server] accepted channel id=" << id << "\n";

            // 수신 시작 (채널 내부에서 on_accept 호출)
            channel->start();

            // 다음 accept
            do_accept();
        });
}

// 예시: 연결 상태 확인, 타임아웃 확인, 유휴 처리 등
void asio_server::tick() {
    std::lock_guard<std::mutex> lock(channel_mutex_);
    for (auto it = channels_.begin(); it != channels_.end(); ) 
    {
        auto& channel = it->second;
        if (channel->is_closed()) 
        {
            it = channels_.erase(it);
        } else 
        {
            ++it;
        }
    }
}

} // namespace lemondory::network