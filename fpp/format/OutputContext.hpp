#pragma once
#include <fpp/base/IOContext.hpp>

namespace fpp {

class OutputContext : public IOContext {

public:

    using WritedCallback = std::function<bool(const std::uint8_t*,std::size_t)>;

    explicit OutputContext(WritedCallback callback);

private:

    bool writePacket(const std::uint8_t* buf, std::size_t buf_size) override;

private:

    WritedCallback      _writeCallback;

};

} // namespace fpp
