#include "OutputContext.hpp"

namespace fpp {

OutputContext::OutputContext(OutputContext::WritedCallback callback)
    : IOContext(Type::Writable)
    , _writeCallback { std::move(callback) }
{}

bool OutputContext::writePacket(const uint8_t* buf, std::size_t buf_size) {
    return _writeCallback(buf, buf_size);
}


} // namespace fpp
