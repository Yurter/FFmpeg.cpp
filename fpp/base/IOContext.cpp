#include "IOContext.hpp"
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libavformat/avio.h>
}

namespace fpp {

int read(void* opaque, std::uint8_t* buf, int buf_size) {
    auto context { reinterpret_cast<IOContext*>(opaque) };
    const auto result {
        context->readPacket(buf, static_cast<std::size_t>(buf_size))
    };
    return result.success ? static_cast<int>(result.bytesRead) : AVERROR(ERROR_EOF);
}

int write(void* opaque, std::uint8_t* buf, int buf_size) {
    auto context { reinterpret_cast<IOContext*>(opaque) };
    const auto success {
        context->writePacket(buf, static_cast<std::size_t>(buf_size))
    };
    return success ? 0 : -1;
}

std::int64_t seek2(void* /*opaque*/, std::int64_t /*offset*/, int /*whence*/) {
    return -1;
}

IOContext::IOContext(Type type, std::size_t buffer_size) {
    _buffer.resize(buffer_size);
    reset(
        ::avio_alloc_context(
              _buffer.data()
            , static_cast<int>(buffer_size)
            , static_cast<int>(type)
            , this /* opaque */
            , &read
            , &write
            , &seek2
        )
        , [](auto* ctx) { ::av_free(ctx); }
    );
}

IOContext::CbResult IOContext::readPacket(std::uint8_t* /*buf*/, std::size_t /*buf_size*/) {
    return {};
}

bool IOContext::writePacket(const std::uint8_t* /*buf*/, std::size_t /*buf_size*/) {
    return {};
}

bool IOContext::seek(std::int64_t /*offset*/, int /*whence*/) {
    return {};
}

} // namespace fpp
