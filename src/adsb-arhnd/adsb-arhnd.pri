HEADERS_BASE += $$PWD/adsb-arhnd_global.h

HEADERS_ADSB += \
           $$PWD/adsb/adsbstream.h \
           $$PWD/adsb/adsb.h

HEADERS_STREAM += \
           $$PWD/stream/stream.h \
           $$PWD/stream/streamdevice.h

SOURCES += \
           $$PWD/stream/stream.cpp \
           $$PWD/stream/streamdevice.cpp \
           $$PWD/adsb/adsbstream.cpp \
           $$PWD/adsb/adsb.cpp

HEADERS += $$HEADERS_BASE \
           $$HEADERS_ADSB \
           $$HEADERS_STREAM
