QT += core network
include(../../build.pri)
include(adsb-arhnd.pri)

contains(DEFINES, ADSB_ARHND_STATIC) {
    message(Building static adsb-arhnd...)
    CONFIG += staticlib
}

TEMPLATE = lib
TARGET = adsb-arhnd
QT -= gui
QT += serialport

DESTDIR = ../bin
DEFINES += ADSB_ARHND_LIBRARY
MOC_DIR = tmp
OBJECTS_DIR = obj

unix {
	target.path = /usr/lib
	INSTALLS = target
}

header_base.files = $$HEADERS_BASE
header_base.path = /usr/include/adsb-arhnd
INSTALLS += header_base

header_adsb.files = $$HEADERS_ADSB
header_adsb.path = /usr/include/adsb-arhnd/adsb
INSTALLS += header_adsb


unix:!macx: LIBS += -L/usr/lib/ -llog4qt

INCLUDEPATH += /include/
DEPENDPATH += /include/

unix:!macx: LIBS += -L/usr/lib/ -lstream-arhnd

INCLUDEPATH += /usr/include/stream-arhnd
DEPENDPATH += /usr/include/stream-arhnd
