#pragma once
#include <cstdint>

namespace fpp {

class Media {

public:

    enum class Type : std::uint8_t {
          Unknown
        , Video
        , Audio
        , Data       ///< Opaque data information usually continuous
        , Subtitle
        , Attachment ///< Opaque data information usually sparse
        , EndOF
    };

    explicit Media(Type type)
        : _type { type }
    {}

    Type                type() const { return _type; }
    void                setType(Type type) { _type = type; }

    bool                isVideo() const { return typeIs(Type::Video); }
    bool                isAudio() const { return typeIs(Type::Audio); }
    bool                isEOF()   const { return typeIs(Type::EndOF); }

    bool                typeIs(Type media_type) const { return _type == media_type; }

private:

    Type                _type;

};

} // namespace fpp
