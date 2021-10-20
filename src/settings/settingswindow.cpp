#include "settingswindow.h"

#include "commandssettingspage.h"
#include "folderssettingspage.h"
#include "generalsettingspage.h"
#include "settings.h"

#include <KLocalizedString>

#include <QPushButton>

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

//    setCurrentPage(commandsPage);

    connect(foldersSettingsPage, &FoldersSettingsPage::changed, this, [=]() {
        m_changed = true;
        updateButtons();
    });

    connect(commandsSettingsPage, &CommandsSettingsPage::changed, this, [=]() {
        m_changed = true;
        updateButtons();
    });

    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, [=]() {
        foldersSettingsPage->save();
        commandsSettingsPage->save();
        updateButtons();
    });

    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [=]() {
        if (hasChanged()) {
            foldersSettingsPage->save();
            commandsSettingsPage->save();
            updateButtons();
        }
    });
}

bool SettingsWindow::hasChanged()
{
    if (m_changed) {
        return true;
    }
    return KConfigDialog::hasChanged();
}
