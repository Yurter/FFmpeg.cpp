TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

#INCLUDEPATH += ../00_ffmpeg/ffmpeg-4.1.3-win64-dev/include
#LIBS += -L../../00_ffmpeg/ffmpeg-4.1.3-win64-dev/lib

#INCLUDEPATH += G:\dev\00_ffmpeg\ffmpeg-4.1.1-win64-dev\include
#LIBS += -LG:\dev\00_ffmpeg\ffmpeg-4.1.1-win64-dev\lib

#INCLUDEPATH += D:\dev\00_ffmpeg\ffmpeg-4.1.3-win64-dev\include
#LIBS += -LD:\dev\00_ffmpeg\ffmpeg-4.1.3-win64-dev\lib

INCLUDEPATH += G:\dev\00_ffmpeg\ffmpeg-4.2.2-win64-dev\include
LIBS += -LG:\dev\00_ffmpeg\ffmpeg-4.2.2-win64-dev\lib

LIBS += -lavcodec -lavdevice -lavfilter -lavformat -lavutil
LIBS += -lpostproc -lswresample -lswscale

INCLUDEPATH += include

SOURCES += \
    examples/adaptive_streaming.cpp \
    examples/concatenate.cpp \
    examples/filter_text_on_video.cpp \
    examples/filter_timelapase.cpp \
    examples/record_screen_win.cpp \
    examples/rtp_audio_stream.cpp \
    examples/rtp_video_and_audio_stream.cpp \
    examples/rtp_video_stream.cpp \
    examples/rtp_video_stream_transcoded.cpp \
    examples/transmuxing.cpp \
    examples/transrating.cpp \
    examples/transsizing.cpp \
    examples/webcam.cpp \
    examples/youtube_stream_copy.cpp \
    examples/youtube_stream_copy_with_silence.cpp \
    examples/youtube_stream_transcode.cpp \
    fpp/refi/VideoFilters/DrawText.cpp \
    fpp/refi/ResampleContext.cpp \
    main.cpp \
    fpp/base/CodecContext.cpp \
    fpp/base/Dictionary.cpp \
    fpp/base/FormatContext.cpp \
    fpp/base/Frame.cpp \
    fpp/base/Packet.cpp \
    fpp/base/Parameters.cpp \
    fpp/codec/DecoderContext.cpp \
    fpp/codec/EncoderContext.cpp \
    fpp/context/InputFormatContext.cpp \
    fpp/context/OutputFormatContext.cpp \
    fpp/core/FFmpegException.cpp \
    fpp/core/Logger.cpp \
    fpp/core/Object.cpp \
    fpp/core/Utils.cpp \
    fpp/refi/AudioFilterContext.cpp \
    fpp/base/FilterContext.cpp \
    fpp/refi/RescaleContext.cpp \
    fpp/refi/VideoFilterContext.cpp \
    fpp/stream/AudioParameters.cpp \
    fpp/stream/Stream.cpp \
    fpp/stream/VideoParameters.cpp

HEADERS += \
    examples/examples.hpp \
    fpp/base/CodecContext.hpp \
    fpp/base/Dictionary.hpp \
    fpp/base/FormatContext.hpp \
    fpp/base/Frame.hpp \
    fpp/base/MediaData.hpp \
    fpp/base/Packet.hpp \
    fpp/base/Parameters.hpp \
    fpp/codec/DecoderContext.hpp \
    fpp/codec/EncoderContext.hpp \
    fpp/context/InputFormatContext.hpp \
    fpp/context/OutputFormatContext.hpp \
    fpp/core/FFmpegException.hpp \
    fpp/core/Logger.hpp \
    fpp/core/Object.hpp \
    fpp/core/Utils.hpp \
    fpp/core/async/AsyncList.hpp \
    fpp/core/async/AsyncObject.hpp \
    fpp/core/async/AsyncQueue.hpp \
    fpp/core/async/AsyncVector.hpp \
    fpp/core/time/Chronometer.hpp \
    fpp/core/time/Timer.hpp \
    fpp/core/wrap/FFmpegObject.hpp \
    fpp/core/wrap/SharedFFmpegObject.hpp \
    fpp/refi/AudioFilterContext.hpp \
    fpp/refi/VideoFilters/DrawText.hpp \
    fpp/base/FilterContext.hpp \
    fpp/refi/ResampleContext.hpp \
    fpp/refi/RescaleContext.hpp \
    fpp/refi/VideoFilterContext.hpp \
    fpp/stream/AudioParameters.hpp \
    fpp/stream/Stream.hpp \
    fpp/stream/VideoParameters.hpp
