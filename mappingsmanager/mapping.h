#ifndef MAPPING_H
#define MAPPING_H

#pragma once

#include <coreplugin/id.h>
#include <utils/fileutils.h>

#include <QMap>
#include <QStandardItemModel>
#include <QString>
#include <QVariant>

namespace ProjectExplorer {
class Project;
}

namespace RemoteDev {
namespace Internal {

//! A handy wrapper around MappingsManager internal storage
class Mapping
{
public:
    //! Check if this mapping is enabled in settings
    bool isEnabled() const;

    //! Name of this mapping
    QString name() const;

    //! Device ID for this mapping (device may not exist)
    Core::Id deviceId() const;

    //! Remote path for this mapping
    Utils::FileName remotePath() const;

private:
    friend class MappingsManager;

    // These are for MappingsManager only
    Mapping(int index, QStandardItemModel &storage);
    Mapping(const QString &name,
            bool isEnabled,
            const Core::Id &deviceId,
            const QString &remotePath,
            QStandardItemModel &storage);

    Mapping() = delete;

    //! Restore Mapping from QVariantMap (settings)
    //!
    //! \param[in]     map          settings for new Mapping
    //! \param[in,out] mappings     storage to place new Mapping in
    //!
    //! \todo make this method not require the storage
    //!
    //! \return Mapping access object
    static Mapping fromMap(const QVariantMap &map, QStandardItemModel &storage);

    //! Serialize into a QVariantMap (settings)
    QVariantMap toMap() const;

private:
    int m_index;
    QStandardItemModel *m_storage;
};

} // namespace Internal
} // namespace RemoteDev

#endif // MAPPING_H
