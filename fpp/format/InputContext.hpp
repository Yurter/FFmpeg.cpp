#pragma once
#include <fpp/base/IOContext.hpp>

namespace fpp {

class InputContext : public IOContext {

public:

    using ReadCallback = std::function<CbResult(std::uint8_t*,std::size_t)>;

    explicit InputContext(ReadCallback callback, std::size_t buffer_size = 4096);

private:

    CbResult readPacket(std::uint8_t* buf, std::size_t buf_size) override;

private:

    ReadCallback        _readCallback;

};

} // namespace fpp
