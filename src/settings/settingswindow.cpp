#include "settingswindow.h"

#include "commandssettingspage.h"
#include "folderssettingspage.h"
#include "generalsettingspage.h"

#include <KLocalizedString>

#include <QPushButton>

SettingsWindow::SettingsWindow(QWidget *parent, KConfigSkeleton *skeleton)
    : KConfigDialog(parent, QStringLiteral("settings"), skeleton)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Settings"));
    resize(650, 550);

    m_config = KSharedConfig::openConfig(u"quickaccessrc"_qs);

    auto generalSettingsPage = new GeneralSettingsPage(this);
    addPage(generalSettingsPage, i18n("General"), QStringLiteral("configure"), QString());
    connect(generalSettingsPage, &GeneralSettingsPage::openStartUpDialog,
            this, &SettingsWindow::openStartUpDialog);

    auto foldersSettingsPage = new FoldersSettingsPage(this);
    addPage(foldersSettingsPage, i18n("Folders"), QStringLiteral("folder"), QString());

    auto commandsSettingsPage = new CommandsSettingsPage(this);
    addPage(commandsSettingsPage, i18n("Commands"), QStringLiteral("dialog-scripts"), QString());

//    setCurrentPage(commandsPage);

    connect(foldersSettingsPage, &FoldersSettingsPage::changed, this, [=]() {
        // enable apply buttton
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
        // disable apply buttton
        m_changed = false;
        updateButtons();
        Q_EMIT settingsChanged(QStringLiteral("settings"));
    });

    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [=]() {
        if (hasChanged()) {
            foldersSettingsPage->save();
            commandsSettingsPage->save();
            updateButtons();
            Q_EMIT settingsChanged(QStringLiteral("settings"));
        }
    });
}

bool SettingsWindow::hasChanged()
{
    if (m_changed) {
        Q_EMIT changed();
        return true;
    }
    return KConfigDialog::hasChanged();
}
