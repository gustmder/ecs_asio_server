#include "common/network/socket_channel_base.hpp"

namespace lemondory::network {

socket_channel_base::socket_channel_base() = default;

socket_channel_base::~socket_channel_base() = default;

void socket_channel_base::on_connect(net_error net_error) {
    if (handler_) {
        handler_->on_connect(net_error);
    }
}

void socket_channel_base::on_accept() {
    if (handler_) {
        handler_->on_accept(channel_id_, this);
    }
}

void socket_channel_base::on_close(const void* error, close_function close_function) {
    if (handler_) {
        handler_->on_close(channel_id_, error, close_function);
    }
    if (close_callback_) {
        close_callback_(error, close_function);
    }
}

} // namespace lemondory::network
