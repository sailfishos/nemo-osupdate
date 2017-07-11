TEMPLATE = lib
TARGET = nemoosupdate

CONFIG += \
        c++11 \
        hide_symbols \
        link_pkgconfig \
        create_pc \
        create_prl \
        no_install_prl

QT -= gui

DEFINES += NEMO_BUILD_OSUPDATE_LIBRARY

PUBLIC_HEADERS += \
        global.h

HEADERS += $$PUBLIC_HEADERS \

public_headers.files = $$PUBLIC_HEADERS
public_headers.path = /usr/include/nemo-osupdate

target.path = /usr/lib

QMAKE_PKGCONFIG_NAME = nemoosupdate
QMAKE_PKGCONFIG_DESCRIPTION = Nemo library for OS Update
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = /usr/include
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += \
        public_headers \
        target
