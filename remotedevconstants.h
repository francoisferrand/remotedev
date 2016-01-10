#ifndef REMOTEDEVCONSTANTS_H
#define REMOTEDEVCONSTANTS_H

#include <Qt>

namespace RemoteDev {
namespace Constants {

/**
 * @brief ACTION_ID identifies RemoteDev default action (to be removed)
 */
const char ACTION_ID[] = "RemoteDev.Action";

const char UPLOAD_FILE[]        = "RemoteDev.UploadFile";
const char UPLOAD_DIRECTORY[]   = "RemoteDev.UploadDirectory";

/**
 * @brief MENU_ID   identifies RemoteDev default menu
 */
const char MENU_ID[] = "RemoteDev.Menu";
/**
 * @brief SETTINGS_GROUP plugin's group in QtCreator's settings object
 * Also, used in ProjectExplorer's Project settings
 */
const char SETTINGS_GROUP[] = "RemoteDev";
/**
 * @brief MAPPINGS_GROUP subsection which stores remote mappings for project
 */
const char MAPPINGS_GROUP[] = "Mappings";

/**
 * Column numbers for mappings data model
 * @todo doxygen group
 */
const int  MAP_NAME_COLUMN    = 0;
const int  MAP_ENABLED_COLUMN = 1;
const int  MAP_DEVICE_COLUMN  = 2;
const int  MAP_PATH_COLUMN    = 3;
const int  MAP_COLUMNS_COUNT  = 4;

const int  DEV_NAME_COLUMN    = 0;
const int  DEV_ID_COLUMN      = 1;
const int  DEV_COLUMNS_COUNT  = 2;

const int  DEV_ID_ROLE        = Qt::UserRole + 1;

} // namespace RemoteDev
} // namespace Constants

#endif // REMOTEDEVCONSTANTS_H
