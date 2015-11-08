QTC_PLUGIN_NAME = RemoteDev
QTC_LIB_DEPENDS += \
    ssh utils
    # aggregation extensionsystem

QTC_PLUGIN_DEPENDS += \
    coreplugin projectexplorer

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

# FIXME: should this be moved to *.pro?
QT *= network
