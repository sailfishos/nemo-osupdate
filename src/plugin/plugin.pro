TARGET = nemoosupdate
PLUGIN_IMPORT_PATH = Nemo/OsUpdate

TEMPLATE = lib
CONFIG += qt plugin hide_symbols
QT += qml
LIBS += -L../nemo-osupdate -lnemoosupdate
target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

INCLUDEPATH += $$PWD/.. $$PWD/../nemo-osupdate

qmldir.files += $$_PRO_FILE_PWD_/qmldir
qmldir.path +=  $$target.path
INSTALLS += qmldir

SOURCES += plugin.cpp
