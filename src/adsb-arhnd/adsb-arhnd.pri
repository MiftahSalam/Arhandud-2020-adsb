HEADERS_BASE += $$PWD/adsb-arhnd_global.h

HEADERS_ADSB += \
           $$PWD/adsb/adsbstream.h \
           $$PWD/adsb/adsb.h

SOURCES += \
           $$PWD/adsb/adsbstream.cpp \
           $$PWD/adsb/adsb.cpp

HEADERS += $$HEADERS_BASE \
           $$HEADERS_ADSB
