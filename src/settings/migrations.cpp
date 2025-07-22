#include "migrations.h"

#include "quickaccess-version.h"

#include <KConfigGroup>
#include <QDebug>

using namespace Qt::StringLiterals;

Migrations::Migrations()
{
    m_config = KSharedConfig::openConfig(u"quickaccessrc"_s);
    m_version = m_config->group(u"General"_s).readEntry(u"Version"_s);
}

void Migrations::migrate()
{
    backupConfigFile();
    migrateV2toV3();
}

bool Migrations::migrateV2toV3()
{
    if (m_version.isEmpty() && QUICKACCESS_VERSION_MAJOR == 3) {
        auto generalGroup = m_config->group(u"General"_s);
        auto commandsGroup = m_config->group(u"Commands"_s);
        auto pathsGroup = m_config->group(u"Paths"_s);

        auto entries = pathsGroup.readEntry(u"paths"_s, QStringList());
        int foldersCount = entries.count();

        for (int i = 0; i < foldersCount; ++i) {
            QString path = entries.at(i);
            QString iconName = u"folder"_s;

            auto group = m_config->group(u"Folder_%1"_s.arg(i));
            group.writeEntry(u"Path"_s, path);
            group.writeEntry(u"Icon"_s, iconName);
        }

        generalGroup.writeEntry("CommandsCount",
                                commandsGroup.readEntry("Count"));

        generalGroup.writeEntry("SubmenuEntriesCount",
                                commandsGroup.readEntry("SubmenuEntriesCount"));

        generalGroup.writeEntry("FoldersCount",
                                QString::number(foldersCount));

        generalGroup.writeEntry("Version",
                                QUICKACCESS_VERSION_STRING);

        m_config->deleteGroup(u"Paths"_s);
        m_config->deleteGroup(u"Commands"_s);

        return m_config->sync();
    }
    return false;
}

bool Migrations::backupConfigFile()
{
    auto newCfg = m_config->copyTo(u"quickaccessrc.backup"_s);
    return newCfg->sync();
}
