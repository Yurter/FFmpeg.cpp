#include "InputContext.hpp"

namespace fpp {

InputContext::InputContext(InputContext::ReadCallback callback, std::size_t buffer_size)
    : IOContext(Type::Readable, buffer_size)
    , _readCallback { std::move(callback) }
{}

IOContext::CbResult InputContext::readPacket(uint8_t* buf, std::size_t buf_size) {
    return _readCallback(buf, buf_size);
}

} // namespace fpp
