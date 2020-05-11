#pragma once
#include <stdint.h>

namespace fpp {

    enum class MediaType : uint8_t {
        Unknown,
        Video,
        Audio,
        Data,       ///< Opaque data information usually continuous
        Subtitle,
        Attachment, ///< Opaque data information usually sparse
        EndOF,
    };

    class MediaData { // TODO: rename class (remove 'Data' from name) (10.05)

    public:

        explicit MediaData(MediaType type)
            : _type(type) {
        }

        MediaType           type() const { return _type; }
        void                setType(MediaType type) { _type = type; }

        bool                isVideo() const { return _type == MediaType::Video; }
        bool                isAudio() const { return _type == MediaType::Audio; }
        bool                isEOF()   const { return _type == MediaType::EndOF; }

        bool                typeIs(MediaType media_type) const { return _type == media_type; }

    private:

        MediaType           _type;

    };

} // namespace fpp
