#ifndef REMOTEDEVCONSTANTS_H
#define REMOTEDEVCONSTANTS_H

#pragma once

#include <Qt>

namespace RemoteDev {
namespace Constants {

/**
 * @brief ACTION_ID identifies RemoteDev default action (to be removed)
 */
extern const char ACTION_ID[];

extern const char UPLOAD_FILE[];
extern const char DOWNLOAD_FILE[];

extern const char UPLOAD_DIRECTORY[];

/**
 * @brief MENU_ID   identifies RemoteDev default menu
 */
extern const char MENU_ID[];
/**
 * @brief SETTINGS_GROUP plugin's group in QtCreator's settings object
 * Also, used in ProjectExplorer's Project settings
 */
extern const char SETTINGS_GROUP[];
/**
 * @brief MAPPINGS_GROUP subsection which stores remote mappings for project
 */
extern const char MAPPINGS_GROUP[];

/**
 * Column numbers for mappings data model
 * @todo doxygen group
 */
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
