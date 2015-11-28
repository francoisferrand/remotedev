#ifndef REMOTEDEVCONSTANTS_H
#define REMOTEDEVCONSTANTS_H

namespace RemoteDev {
namespace Constants {

/**
 * @brief ACTION_ID identifies RemoteDev default action (to be removed)
 */
const char ACTION_ID[] = "RemoteDev.Action";
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

} // namespace RemoteDev
} // namespace Constants

#endif // REMOTEDEVCONSTANTS_H
