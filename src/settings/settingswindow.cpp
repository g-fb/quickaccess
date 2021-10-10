#include "settingswindow.h"

#include "commandssettingspage.h"
#include "folderssettingspage.h"
#include "generalsettingspage.h"

#include <KLocalizedString>

SettingsWindow::SettingsWindow(QWidget *parent, KConfigSkeleton *skeleton)
    : KConfigDialog(parent, QStringLiteral("settings"), skeleton)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Settings"));
    resize(650, 550);

    m_config = KSharedConfig::openConfig("quickaccessrc");

    auto generalSettingsPage = new GeneralSettingsPage(this);
    auto generalPage = addPage(generalSettingsPage, i18n("General"), QStringLiteral("configure"), QString());

    auto foldersSettingsPage = new FoldersSettingsPage(this);
    auto foldersPage = addPage(foldersSettingsPage, i18n("Folders"), QStringLiteral("folder"), QString());

    auto commandsSettingsPage = new CommandsSettingsPage(this);
    auto commandsPage = addPage(commandsSettingsPage, i18n("Commands"), QStringLiteral("dialog-scripts"), QString());

    setCurrentPage(commandsPage);

    connect(generalSettingsPage, &GeneralSettingsPage::changed, this, [=]() {
        // update buttons state
    });
    connect(foldersSettingsPage, &FoldersSettingsPage::changed, this, [=]() {
        // update buttons state
    });
    connect(commandsSettingsPage, &CommandsSettingsPage::changed, this, [=]() {
        // update buttons state
    });
}
