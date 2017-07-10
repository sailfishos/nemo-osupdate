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
QT += dbus

DEFINES += NEMO_BUILD_OSUPDATE_LIBRARY
PKGCONFIG += systemsettings

system(qdbusxml2cpp org.nemo.osupdater.xml -a osupdateradaptor -c OsUpdaterAdaptor -l Nemo::OsUpdater -i osupdater.h)

PUBLIC_HEADERS += \
        global.h \
        osupdater.h \
        osupdateradaptor.h \
        osupdaterclient.h \
        osupdaterservice.h \

HEADERS += $$PUBLIC_HEADERS \
        osupdater_p.h \

SOURCES += osupdater.cpp \
        osupdateradaptor.cpp \
        osupdaterclient.cpp \
        osupdaterservice.cpp \

DBUS_INTERFACES += org.nemo.osupdater.xml
OTHER_FILES += org.nemo.osupdater.xml

public_headers.files = $$PUBLIC_HEADERS
public_headers.path = /usr/include/nemo-osupdate

target.path = /usr/lib

QMAKE_PKGCONFIG_NAME = nemoosupdate
QMAKE_PKGCONFIG_DESCRIPTION = Nemo library for OS Update
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$public_headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += \
        public_headers \
        target
