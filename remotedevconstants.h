#ifndef REMOTEDEVCONSTANTS_H
#define REMOTEDEVCONSTANTS_H

#pragma once

#include <Qt>

namespace RemoteDev {
namespace Constants {

//! identifies RemoteDev default action (to be removed)
const char ACTION_ID[] = "RemoteDev.Action";

const char UPLOAD_FILE[] = "RemoteDev.UploadFile";
const char DOWNLOAD_FILE[] = "RemoteDev.DownloadFile";

const char UPLOAD_DIRECTORY[] = "RemoteDev.UploadDirectory";

const char G_DOWNLOAD_FILE[] = "RemoteDev.Group.DownloadFile";

//! identifies RemoteDev default menu (to be removed)
const char MENU_ID[] = "RemoteDev.Menu";

//! Identifies RemoteDev menu for download sources
const char M_DOWNLOAD_SOURCES[] = "RemoteDev.Menu.DownloadSources";

//! Name of property of QAction that points to it's mapping
const char P_MAPPING[] = "mapping";

//! Plugin's group in QtCreator's settings object
//!
//! Also, used in ProjectExplorer's Project settings
const char SETTINGS_GROUP[] = "RemoteDev";

//! Subsection which stores remote mappings for project
const char MAPPINGS_GROUP[] = "Mappings";

//! Column numbers for mappings data model
//! \todo doxygen group
constexpr int  MAP_NAME_COLUMN    = 0;
constexpr int  MAP_ENABLED_COLUMN = 1;
constexpr int  MAP_DEVICE_COLUMN  = 2;
constexpr int  MAP_PATH_COLUMN    = 3;
constexpr int  MAP_COLUMNS_COUNT  = 4;

constexpr int  DEV_NAME_COLUMN    = 0;
constexpr int  DEV_ID_COLUMN      = 1;
constexpr int  DEV_COLUMNS_COUNT  = 2;

constexpr int  DEV_ID_ROLE        = Qt::UserRole + 1;

} // namespace RemoteDev
} // namespace Constants

#endif // REMOTEDEVCONSTANTS_H
