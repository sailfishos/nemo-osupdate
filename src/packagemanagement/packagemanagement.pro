TEMPLATE = lib
TARGET = nemoosupdate-packagemanagement

DEFINES += NEMO_BUILD_OSUPDATE_LIBRARY

CONFIG += \
        c++11 \
        hide_symbols \
        link_pkgconfig \
        create_pc \
        create_prl \
        no_install_prl

INCLUDEPATH += $$PWD/..

PKGCONFIG += \
    packagekit-qt5 \
    rpm

LIBS += \
    -lssu

QT -= gui
QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi

SOURCES += \
    collection.cpp \
    manager.cpp \
    transaction.cpp \
    packagekit/distchecktransactionpkgkit.cpp \
    packagekit/distupgradetransactionpkgkit.cpp \
    packagekit/infotransactionrpm.cpp \
    packagekit/installtransactionpkgkit.cpp \
    packagekit/listtransactionpkgkit.cpp \
    packagekit/packagekitmanager.cpp \
    packagekit/osupdatesizetransactionpkgkit.cpp \
    packagekit/packagefilestransactionpkgkit.cpp \
    packagekit/packagenametransactionpkgkit.cpp \
    packagekit/repositorytransactionpkgkit.cpp \
    packagekit/uninstalltransactionpkgkit.cpp

PUBLIC_HEADERS += \
    collection.h \
    manager.h \
    transaction.h \
    packagekit/packagekitmanager.h

HEADERS += \
    $$PUBLIC_HEADERS \
    packagekit/distchecktransactionpkgkit.h \
    packagekit/distupgradetransactionpkgkit.h \
    packagekit/infotransactionrpm.h \
    packagekit/installtransactionpkgkit.h \
    packagekit/listtransactionpkgkit.h \
    packagekit/osupdatesizetransactionpkgkit.h \
    packagekit/packagefilestransactionpkgkit.h \
    packagekit/packagenametransactionpkgkit.h \
    packagekit/repositorytransactionpkgkit.h \
    packagekit/uninstalltransactionpkgkit.h

public_headers.files = $$PUBLIC_HEADERS
public_headers.path = /usr/include/nemo-osupdate/packagemanagement

target.path = /usr/lib

QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = Nemo Library for OS Updates - Package management
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$public_headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += \
        public_headers \
        target
