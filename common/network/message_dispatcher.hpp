#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace lemondory::network 
{

    using msg_handler = std::function<void(int channel_id, const char* data, std::size_t size)>;

    class message_dispatcher 
    {
        public:
            void bind(std::uint16_t msg_id, msg_handler h) { table_[msg_id] = std::move(h); }
            void unbind(std::uint16_t msg_id) { table_.erase(msg_id); }

            void dispatch(std::uint16_t msg_id, int channel_id, const char* data, std::size_t size) const 
            {
                if (auto it = table_.find(msg_id); it != table_.end()) 
                {
                    it->second(channel_id, data, size);
                }
            }

        private:
            std::unordered_map<std::uint16_t, msg_handler> table_;
    };

} // namespace lemondory::network

// #pragma once
// #include <cstdint>
// #include <functional>
// #include <unordered_map>

// namespace lemondory::network {

// using msg_handler = std::function<void(int channel_id, const char* data, std::size_t size)>;

// class message_dispatcher {
// public:
//     void bind(std::uint16_t msg_id, msg_handler h) { table_[msg_id] = std::move(h); }

//     void dispatch(std::uint16_t msg_id, int channel_id,
//                   const char* data, std::size_t size) const {
//         if (auto it = table_.find(msg_id); it != table_.end()) {
//             it->second(channel_id, data, size);
//         }
//     }

// private:
//     std::unordered_map<std::uint16_t, msg_handler> table_;
// };

// } // namespace lemondory::network