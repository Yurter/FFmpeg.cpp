#include "Packet.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

namespace fpp {

Packet::Packet(Type type)
    : Media(type)
    , _time_base { DEFAULT_RATIONAL } {
}

Packet::Packet(const Packet& other)
    : Packet(other.type()) {
    ref(other);
}

Packet::Packet(const AVPacket& avpacket, AVRational time_base, Media::Type type)
    : Packet(type) {
    ref(avpacket, time_base);
}

Packet::~Packet() {
    unref();
}

Packet& Packet::operator=(const Packet& other) {
    unref();
    ref(other);
    setType(other.type());
    return *this;
}

void Packet::setPts(std::int64_t pts) {
    raw().pts = pts;
}

void Packet::setDts(std::int64_t dts) {
    raw().dts = dts;
}

void Packet::setPos(std::int64_t pos) {
    raw().pos = pos;
}

void Packet::setDuration(std::int64_t duration) {
    raw().duration = duration;
}

void Packet::setTimeBase(AVRational time_base) {
    _time_base = time_base;
}

void Packet::setStreamIndex(int stream_index) {
    raw().stream_index = stream_index;
}

int64_t Packet::pts() const {
    return raw().pts;
}

int64_t Packet::dts() const {
    return raw().dts;
}

int64_t Packet::duration() const {
    return raw().duration;
}

AVRational Packet::timeBase() const {
    return _time_base;
}

int64_t Packet::pos() const {
    return raw().pos;
}

int Packet::streamIndex() const {
    return raw().stream_index;
}

bool Packet::keyFrame() const {
    return raw().flags & AV_PKT_FLAG_KEY;
}

int Packet::size() const {
    return raw().size;
}

std::string Packet::toString() const {
    /* Video packet: [I], 1516 bytes, dts 1709, pts 1709, duration 29, time_base 1/90000, stream index 0    */
    /* Audio packet: [I], 1045 bytes, dts 392, pts 392, duration 26, time_base 1/44100, stream index 1      */
    return utils::to_string(type()) + " packet" +
            (!isEOF()
                ? std::string { ": " } + (keyFrame() ? "[I]" : "[_]") + ", "
                    + std::to_string(raw().size) + " bytes, "
                    + "dts " + utils::pts_to_string(raw().dts) + ", "
                    + "pts " + utils::pts_to_string(raw().pts) + ", "
                    + "duration " + std::to_string(raw().duration) + ", "
                    + "time_base " + utils::to_string(_time_base) + ", "
                    + "stream index " + std::to_string(raw().stream_index)
                : "" );
}

void Packet::ref(const Packet& other) {
    ref(other.raw(), other.timeBase());
}

void Packet::ref(const AVPacket& other, AVRational time_base) {
    ffmpeg_api_strict(av_packet_ref, ptr(), &other);
    setTimeBase(time_base);
}

void Packet::unref() {
    ::av_packet_unref(ptr());
}

} // namespace fpp
