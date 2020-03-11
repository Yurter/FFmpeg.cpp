TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

#FFmpeg
INCLUDEPATH += ../00_ffmpeg/ffmpeg-4.1.3-win64-dev/include
LIBS += -L../../00_ffmpeg/ffmpeg-4.1.3-win64-dev/lib
LIBS += -lavcodec -lavdevice -lavfilter -lavformat -lavutil
LIBS += -lpostproc -lswresample -lswscale

INCLUDEPATH += include

SOURCES += \
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
    fpp/core/Thread.cpp \
    fpp/core/Utils.cpp \
    fpp/refi/AudioFilterContext.cpp \
    fpp/refi/FilterContext.cpp \
    fpp/refi/ResamplerContext.cpp \
    fpp/refi/RescalerContext.cpp \
    fpp/refi/VideoFilterContext.cpp \
    fpp/stream/AudioParameters.cpp \
    fpp/stream/Stream.cpp \
    fpp/stream/VideoParameters.cpp

HEADERS += \
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
    fpp/core/Thread.hpp \
    fpp/core/Utils.hpp \
    fpp/core/async/AsyncList.hpp \
    fpp/core/async/AsyncObject.hpp \
    fpp/core/async/AsyncQueue.hpp \
    fpp/core/async/AsyncVector.hpp \
    fpp/core/time/Chronometer.hpp \
    fpp/core/time/Timer.hpp \
    fpp/core/wrap/FFmpegObject.hpp \
    fpp/core/wrap/SemiSmartPointer.hpp \
    fpp/core/wrap/SharedFFmpegObject.hpp \
    fpp/refi/AudioFilterContext.hpp \
    fpp/refi/FilterContext.hpp \
    fpp/refi/ResamplerContext.hpp \
    fpp/refi/RescalerContext.hpp \
    fpp/refi/VideoFilterContext.hpp \
    fpp/stream/AudioParameters.hpp \
    fpp/stream/Stream.hpp \
    fpp/stream/VideoParameters.hpp