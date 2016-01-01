DEFINES += REMOTEDEV_LIBRARY

# RemoteDev files

SOURCES += \
    remotedevplugin.cpp \
    connectionmanager.cpp \
    connection/sftpconnection.cpp \
    connection.cpp \
    connectionspage.cpp \
    iconnectionoptionspage.cpp \
    connection/sftpsettingswidget.cpp \
    connection/sftpoptionspage.cpp \
    projectsettingswidget.cpp \
    mappingsmanager.cpp \
    devicemanager.cpp \
    settingswidget.cpp

HEADERS += \
    remotedevplugin.h \
    remotedev_global.h \
    remotedevconstants.h \
    connectionmanager.h \
    connection/sftpconnection.h \
    connection.h \
    connectionspage.h \
    iconnectionoptionspage.h \
    connection/sftpsettingswidget.h \
    connection/sftpoptionspage.h \
    connectionconstants.h \
    projectsettingswidget.h \
    mappingsmanager.h \
    devicemanager.h \
    settingswidget.h

CONFIG += C++11

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
    connection/sftpsettingswidget.ui \
    projectsettingswidget.ui \
    settingswidget.ui

