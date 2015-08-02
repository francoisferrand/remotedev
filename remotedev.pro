DEFINES += REMOTEDEV_LIBRARY

# RemoteDev files

SOURCES += remotedevplugin.cpp \
    connectionmanager.cpp \
    remoteconnection.cpp \
    remoteconnection/sftpconnection.cpp

HEADERS += remotedevplugin.h \
        remotedev_global.h \
        remotedevconstants.h \
    connectionmanager.h \
    remoteconnection.h \
    remoteconnection/sftpconnection.h

CONFIG += C++11

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=/home/elvenfighter/Projects/qt-creator

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=/home/elvenfighter/Programs/QtTest/Tools/QtCreator

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
# USE_USER_DESTDIR = yes

include(remotedev_dependencies.pri)

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

