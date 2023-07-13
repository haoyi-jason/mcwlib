#-------------------------------------------------
#
# Project created by QtCreator 2023-07-13T13:34:40
#
#-------------------------------------------------

QT       += core network serialbus serialport

QT       -= gui

TARGET = mcwlib
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = "$$PWD/output"


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += HY_EXPORT_LIB

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        mcwlib.cpp \
        protocol/canbusprotocol.cpp \
        protocol/consolelogger.cpp \
        protocol/protocolcanbus.cpp \
        protocol/protocoldriver.cpp \
        protocol/protocolserial.cpp \
        protocol/protocoludp.cpp \
        protocol/rcuprotocol.cpp \
        utility/settings.cpp

HEADERS += \
        mcwlib.h \
        protocol/canbusprotocol.h \
        protocol/consolelogger.h \
        protocol/protocolcanbus.h \
        protocol/protocoldriver.h \
        protocol/protocolserial.h \
        protocol/protocoludp.h \
        protocol/rcuprotocol.h \
        utility/settings.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += D:\workDir\BMS\UDTStudio_fork\src\lib\canopen\
