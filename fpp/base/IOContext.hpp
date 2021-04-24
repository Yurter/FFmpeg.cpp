#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>

struct AVIOContext;

namespace fpp {

class IOContext : public SharedFFmpegObject<AVIOContext> {

public:

    enum class Type {
          Readable = 0
        , Writable = 1
    };

    struct CbResult {
        const bool success { false };
        const std::size_t bytesRead { 0 };
    };

    IOContext(Type type, std::size_t buffer_size = 4096);

protected:

    friend int read(void* opaque, std::uint8_t* buf, int buf_size);
    friend int write(void* opaque, std::uint8_t* buf, int buf_size);
    friend std::int64_t seek2(void* opaque, std::int64_t offset, int whence);

    virtual CbResult readPacket(std::uint8_t* buf, std::size_t buf_size);
    virtual bool writePacket(const std::uint8_t* buf, std::size_t buf_size);
    virtual bool seek(std::int64_t offset, int whence);

private:

    std::vector<std::uint8_t> _buffer;

};

} // namespace fpp
