// simple_client.cpp
// 서버 동작 확인용 데모 클라이언트
// 빌드: g++ -std=c++17 -I third_party/asio/asio/include -DASIO_STANDALONE simple_client.cpp -o simple_client
//
// 흐름: 접속 → LOGIN → MOVE → CHAT → PING/PONG 수신 대기

#include "common/network/frame_codec.hpp"
#include <asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using tcp = asio::ip::tcp;
using namespace lemondory::network::frame_codec;

// 단일 프레임 동기 전송
static void send_frame(tcp::socket& sock, std::uint16_t cmd, const std::string& payload) {
    auto pkt = pack(cmd, 0, k_proto_ver, 0, std::string_view{payload});
    asio::write(sock, asio::buffer(pkt));
    std::cout << "[send] cmd=" << cmd << " payload=\"" << payload << "\"\n";
}

// 수신 루프 (별도 스레드)
static void recv_loop(tcp::socket& sock, std::atomic<bool>& running) {
    std::vector<char> acc;
    acc.reserve(64 * 1024);
    std::size_t acc_offset = 0;
    char buf[4096];

    while (running) {
        asio::error_code ec;
        std::size_t n = sock.read_some(asio::buffer(buf), ec);
        if (ec) break;

        acc.insert(acc.end(), buf, buf + n);

        for (;;) {
            header_v1 hdr{};
            std::string_view payload;
            bool crc_ok = false;
            if (!try_parse_one(acc, acc_offset, hdr, payload, crc_ok)) break;
            if (!crc_ok) continue;

            std::cout << "[recv] cmd=" << hdr.cmd
                      << " payload=\"" << std::string(payload) << "\"\n";
        }

        if (acc_offset > acc.size() / 2) {
            acc.erase(acc.begin(), acc.begin() + static_cast<std::ptrdiff_t>(acc_offset));
            acc_offset = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
    const std::string port = (argc > 2) ? argv[2] : "12345";

    try {
        asio::io_context io;
        tcp::socket sock(io);
        tcp::resolver resolver(io);

        std::cout << "서버 접속 중... " << host << ":" << port << "\n";
        asio::connect(sock, resolver.resolve(host, port));
        std::cout << "접속 완료\n\n";

        std::atomic<bool> running{true};
        std::thread recv_thr([&]{ recv_loop(sock, running); });

        // 1. 로그인
        send_frame(sock, 1001, "TestPlayer");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // 2. 이동 (x=10, y=0, z=5 — float 직렬화)
        {
            float x = 10.f, y = 0.f, z = 5.f;
            std::string move_payload(12, '\0');
            std::memcpy(move_payload.data() + 0, &x, 4);
            std::memcpy(move_payload.data() + 4, &y, 4);
            std::memcpy(move_payload.data() + 8, &z, 4);
            send_frame(sock, 1002, move_payload);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // 3. 채팅
        send_frame(sock, 1003, "안녕하세요!");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // 4. PING (서버 자동 응답 확인)
        send_frame(sock, 1, "");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        running = false;
        asio::error_code ignored;
        sock.shutdown(tcp::socket::shutdown_both, ignored);
        sock.close(ignored);
        recv_thr.join();

        std::cout << "\n연결 종료\n";

    } catch (const std::exception& e) {
        std::cerr << "오류: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
