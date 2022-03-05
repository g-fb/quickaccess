#include "migrations.h"

#include "quickaccess-version.h"

#include <KConfigGroup>
#include <QDebug>

Migrations::Migrations()
{
    m_config = KSharedConfig::openConfig("quickaccessrc");
    m_version = m_config->group(QStringLiteral("General")).readEntry(QStringLiteral("Version"));
}

void Migrations::migrate()
{
    backupConfigFile();
    migrateV2toV3();
}

bool Migrations::migrateV2toV3()
{
    if (m_version.isEmpty() && QUICKACCESS_VERSION_MAJOR == 3) {
        auto generalGroup = m_config->group(QStringLiteral("General"));
        auto commandsGroup = m_config->group(QStringLiteral("Commands"));
        auto pathsGroup = m_config->group(QStringLiteral("Paths"));

        auto entries = pathsGroup.readEntry(QStringLiteral("paths"), QStringList());
        int foldersCount = entries.count();

        for (int i = 0; i < foldersCount; ++i) {
            QString path = entries.at(i);
            QString iconName = QStringLiteral("folder");

            auto group = m_config->group(QString("Folder_%1").arg(i));
            group.writeEntry(QStringLiteral("Path"), path);
            group.writeEntry(QStringLiteral("Icon"), iconName);
        }

        generalGroup.writeEntry(QStringLiteral("CommandsCount"),
                                commandsGroup.readEntry(QStringLiteral("Count")));

        generalGroup.writeEntry(QStringLiteral("SubmenuEntriesCount"),
                                commandsGroup.readEntry(QStringLiteral("SubmenuEntriesCount")));

        generalGroup.writeEntry(QStringLiteral("FoldersCount"),
                                QString::number(foldersCount));

        generalGroup.writeEntry(QStringLiteral("Version"),
                                QUICKACCESS_VERSION_STRING);

        m_config->deleteGroup(QStringLiteral("Paths"));
        m_config->deleteGroup(QStringLiteral("Commands"));

        return m_config->sync();
    }
    return false;
}

bool Migrations::backupConfigFile()
{
    auto newCfg = m_config->copyTo("quickaccessrc.backup");
    return newCfg->sync();
}
