DEFINES += REMOTEDEV_LIBRARY

# RemoteDev files

SOURCES += \
    remotedevplugin.cpp \
    connectionmanager.cpp \
    connection/sftpconnection.cpp \
    connection.cpp \
    projectsettingswidget.cpp \
    mappingsmanager.cpp \
    devicemanager.cpp \
    connectionhelper.cpp \
    optionspage.cpp \
    optionswidget.cpp \
    connection/sftpchannelexecutor.cpp

HEADERS += \
    remotedevplugin.h \
    remotedev_global.h \
    remotedevconstants.h \
    connectionmanager.h \
    connection/sftpconnection.h \
    connection.h \
    connectionconstants.h \
    projectsettingswidget.h \
    mappingsmanager.h \
    devicemanager.h \
    connectionhelper.h \
    optionspage.h \
    optionswidget.h \
    connection/sftpchannelexecutor.h

CONFIG += C++14

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=/home/elvenfighter/Projects/qt-creator

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
#isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=/home/elvenfighter/Programs/QtTest/Tools/QtCreator
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=/home/elvenfighter/Projects/qt-creator-build

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
# USE_USER_DESTDIR = yes

include(remotedev_dependencies.pri)

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

FORMS += \
    projectsettingswidget.ui \
    optionswidget.ui

