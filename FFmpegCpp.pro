#TEMPLATE = lib
#CONFIG += staticlib
#CONFIG += dll

# TODO: build both lib and dll at once

CONFIG(debug, debug|release) {
    TARGET = fppd
} else {
    TARGET = fpp
}

CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

#INCLUDEPATH += ../00_ffmpeg/ffmpeg-4.1.3-win64-dev/include
#LIBS += -L../../00_ffmpeg/ffmpeg-4.1.3-win64-dev/lib

#INCLUDEPATH += G:\dev\00_ffmpeg\ffmpeg-4.1.1-win64-dev\include
#LIBS += -LG:\dev\00_ffmpeg\ffmpeg-4.1.1-win64-dev\lib

#INCLUDEPATH += D:\dev\00_ffmpeg\ffmpeg-4.1.3-win64-dev\include
#LIBS += -LD:\dev\00_ffmpeg\ffmpeg-4.1.3-win64-dev\lib

#INCLUDEPATH += /usr/include/x86_64-linux-gnu/
#LIBS += -L/usr/lib/x86_64-linux-gnu/

#INCLUDEPATH += G:\dev\00_ffmpeg\ffmpeg-4.2.2-win64-dev\include
#LIBS += -LG:\dev\00_ffmpeg\ffmpeg-4.2.2-win64-dev\lib

INCLUDEPATH += D:\libs\ffmpeg\4.1.3\ffmpeg-4.1.3-win64-dev\include
LIBS += -LD:\libs\ffmpeg\4.1.3\ffmpeg-4.1.3-win64-dev\lib

LIBS += -lavcodec -lavdevice -lavfilter -lavformat -lavutil
LIBS += -lpostproc -lswresample -lswscale

INCLUDEPATH += include

SOURCES += \
    examples/adaptive_streaming.cpp \
    examples/concatenate.cpp \
    examples/filter_complex.cpp \
    examples/filter_text_on_video.cpp \
    examples/filter_timelapase.cpp \
    examples/mic_to_file.cpp \
    examples/multiple_outputs_parallel.cpp \
    examples/multiple_outputs_sequence.cpp \
    examples/read_and_write_to_memory.cpp \
    examples/record_screen_win.cpp \
    examples/rtp_audio_stream.cpp \
    examples/rtp_video_and_audio_stream.cpp \
    examples/rtp_video_stream.cpp \
    examples/rtp_video_stream_transcoded.cpp \
    examples/transmuxing.cpp \
    examples/transrating.cpp \
    examples/transsizing.cpp \
    examples/webcam_to_file.cpp \
    examples/webcam_to_udp.cpp \
    examples/write_to_memory.cpp \
    examples/youtube_stream_copy.cpp \
    examples/youtube_stream_copy_with_silence.cpp \
    examples/youtube_stream_transcode.cpp \
    examples/youtube_stream_transcode_with_silence.cpp \
    fpp/base/FilterChain.cpp \
    fpp/base/FilterGraph.cpp \
    fpp/base/IOContext.cpp \
    fpp/filter/BitStreamFilterContext.cpp \
    fpp/filter/ComplexFilterGraph.cpp \
    fpp/filter/LinearFilterGraph.cpp \
    fpp/format/InputContext.cpp \
    fpp/format/OutputContext.cpp \
    fpp/resample/ResampleContext.cpp \
    fpp/refi/VideoFilters/Drawtext.cpp \
    main.cpp \
    fpp/base/CodecContext.cpp \
    fpp/base/Dictionary.cpp \
    fpp/base/FormatContext.cpp \
    fpp/base/Frame.cpp \
    fpp/base/Packet.cpp \
    fpp/base/Parameters.cpp \
    fpp/codec/DecoderContext.cpp \
    fpp/codec/EncoderContext.cpp \
    fpp/format/InputFormatContext.cpp \
    fpp/format/OutputFormatContext.cpp \
    fpp/core/FFmpegException.cpp \
    fpp/core/Logger.cpp \
    fpp/core/Object.cpp \
    fpp/core/Utils.cpp \
    fpp/base/FilterContext.cpp \
    fpp/scale/RescaleContext.cpp \
    fpp/stream/AudioParameters.cpp \
    fpp/stream/Stream.cpp \
    fpp/stream/VideoParameters.cpp

HEADERS += \
    examples/examples.hpp \
    fpp/base/CodecContext.hpp \
    fpp/base/Dictionary.hpp \
    fpp/base/FilterChain.hpp \
    fpp/base/FilterGraph.hpp \
    fpp/base/FormatContext.hpp \
    fpp/base/Frame.hpp \
    fpp/base/IOContext.hpp \
    fpp/base/MediaData.hpp \
    fpp/base/Packet.hpp \
    fpp/base/Parameters.hpp \
    fpp/codec/DecoderContext.hpp \
    fpp/codec/EncoderContext.hpp \
    fpp/format/InputContext.hpp \
    fpp/format/InputFormatContext.hpp \
    fpp/format/OutputContext.hpp \
    fpp/format/OutputFormatContext.hpp \
    fpp/core/FFmpegException.hpp \
    fpp/core/Logger.hpp \
    fpp/core/Object.hpp \
    fpp/core/Utils.hpp \
    fpp/core/time/Chronometer.hpp \
    fpp/core/wrap/FFmpegObject.hpp \
    fpp/core/wrap/SharedFFmpegObject.hpp \
    fpp/filter/BitStreamFilterContext.hpp \
    fpp/filter/ComplexFilterGraph.hpp \
    fpp/filter/LinearFilterGraph.hpp \
    fpp/refi/VideoFilters/DrawText.hpp \
    fpp/base/FilterContext.hpp \
    fpp/resample/ResampleContext.hpp \
    fpp/scale/RescaleContext.hpp \
    fpp/stream/AudioParameters.hpp \
    fpp/stream/Stream.hpp \
    fpp/stream/VideoParameters.hpp
