#include "settingswindow.h"

#include <QPushButton>

#include <KLocalizedString>

#include "commandssettingspage.h"
#include "folderssettingspage.h"
#include "generalsettingspage.h"

using namespace Qt::StringLiterals;

SettingsWindow::SettingsWindow(QWidget *parent, KConfigSkeleton *skeleton)
    : KConfigDialog(parent, u"settings"_s, skeleton)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Settings"));
    resize(650, 550);

    m_config = KSharedConfig::openConfig(u"quickaccessrc"_s);

    auto generalSettingsPage = new GeneralSettingsPage(this);
    addPage(generalSettingsPage, i18n("General"), u"configure"_s, QString());
    connect(generalSettingsPage, &GeneralSettingsPage::openStartUpDialog,
            this, &SettingsWindow::openStartUpDialog);

    auto foldersSettingsPage = new FoldersSettingsPage(this);
    addPage(foldersSettingsPage, i18n("Folders"), u"folder"_s, QString());

    auto commandsSettingsPage = new CommandsSettingsPage(this);
    addPage(commandsSettingsPage, i18n("Commands"), u"dialog-scripts"_s, QString());

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
        Q_EMIT settingsChanged(u"settings"_s);
    });

    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [=]() {
        if (hasChanged()) {
            foldersSettingsPage->save();
            commandsSettingsPage->save();
            updateButtons();
            Q_EMIT settingsChanged(u"settings"_s);
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
