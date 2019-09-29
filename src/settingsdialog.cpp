#include "settingsdialog.h"
#include "settings.h"
#include "mainwindow.h"

SettingsDialog::SettingsDialog(QWidget *parent, const QString &name, KConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
    , m_config(KSharedConfig::openConfig("quickaccessrc"))
    , m_settings(new Settings(this))
    , m_changed(false)
{
    addPage(m_settings, i18n("Settings"));

    auto mainWindow = qobject_cast<MainWindow *>(parent);
    connect(mainWindow, &MainWindow::addFolder, this, [=](QString path) {
        m_settings->kcfg_paths->insertItem(path, m_settings->kcfg_paths->count());
    });

    m_settings->submenuEntriesCountInfo->setText(
                i18n("Use %1 to show all or %2 to show none").arg("-1").arg(0));

    // add button to open file dialog to select a folder
    // and add it to the folders list
    auto addFolderButton = new QPushButton(i18n("Select and Add Folder"));
    addFolderButton->setIcon(QIcon::fromTheme("folder-add"));
    connect(addFolderButton, &QPushButton::clicked, this, [=]() {
        mainWindow->selectFolder();
        emit m_settings->kcfg_paths->changed();
    });
    auto widget = new QWidget();
    auto hLayout = new QHBoxLayout(widget);
    hLayout->setMargin(0);
    hLayout->addWidget(addFolderButton);
    hLayout->addStretch(1);
    // add widget to the keditlistwidget's layout
    m_settings->kcfg_paths->layout()->addWidget(widget);

    auto commandsCount = m_config->group("Commands").readEntry("Count");
    for (int i = 0; i < commandsCount.toInt(); i++) {
        auto command = m_config->group(QString("Command_%1").arg(i));
        auto item = new QTreeWidgetItem();
        item->setText(0, command.readEntry("Name"));
        item->setIcon(0, QIcon::fromTheme(command.readEntry("Icon")));
        item->setText(1, command.readEntry("Command"));
        m_settings->commandsTree->insertTopLevelItem(0, item);
    }

    m_settings->treeItemIcon->setText("media-playback-start-symbolic");
    m_settings->treeItemName->setText("Play");
    m_settings->command->setText("vlc");

    connect(m_settings->addTreeItem, &QPushButton::clicked, this, [=]() {
        auto item = new QTreeWidgetItem();
        item->setIcon(0, QIcon::fromTheme(m_settings->treeItemIcon->text()));
        item->setText(0, m_settings->treeItemName->text());
        if (!m_settings->treeItemIsFolder->isChecked()) {
            item->setText(1, m_settings->command->text());
        }
        m_settings->commandsTree->insertTopLevelItem(0, item);
        m_changed = true;
    });
    connect(m_settings->addTreeItem, &QPushButton::clicked, this, &SettingsDialog::updateButtons);
    connect(m_settings->removeTreeItem, &QPushButton::clicked, this, [=]() {
        for (auto item : m_settings->commandsTree->selectedItems()) {
            delete item;
        }
        m_changed = true;
    });
    connect(m_settings->removeTreeItem, &QPushButton::clicked, this, &SettingsDialog::updateButtons);
    connect(m_settings->treeItemIsFolder, &QCheckBox::clicked, this, [=]() {
        if (m_settings->treeItemIsFolder->isChecked()) {
            m_settings->command->setEnabled(false);
        } else {
            m_settings->command->setEnabled(true);
        }
    });
}

void SettingsDialog::deleteCommands()
{
    int commandsCount = m_config->group("Commands").readEntry("Count").toInt();
    // delete all commands
    for (int i = 0; i < commandsCount; i++) {
        auto command = m_config->group(QString("Command_%1").arg(i));
        if (command.readEntry("IsFolder") == "false") {
            // delete sub commands
        }
        m_config->deleteGroup(command.name());
    }
    m_config->deleteGroup("Commands");
    m_config->sync();
}

void SettingsDialog::saveCommands()
{
    int commandsCount = m_settings->commandsTree->model()->rowCount();
    // save the commands
    for (int i = 0; i < commandsCount; i++) {
        auto index = m_settings->commandsTree->model()->index(i, 0);
        auto item = m_settings->commandsTree->model()->itemData(index);
        QString name = item[0].toString();
        QString iconName = item[1].value<QIcon>().name();
        index = m_settings->commandsTree->model()->index(i, 1);
        item = m_settings->commandsTree->model()->itemData(index);
        QString command = item[0].toString();

        auto group = m_config->group(QString("Command_%1").arg(i));
        group.writeEntry("Name", name);
        group.writeEntry("Icon", iconName);
        group.writeEntry("Command", command);
        group.writeEntry("IsFolder", false);
    }
    auto group = m_config->group("Commands");
    group.writeEntry("Count", QString::number(commandsCount));
    m_config->sync();
}

void SettingsDialog::updateSettings()
{
    deleteCommands();
    saveCommands();
    m_changed = false;
    emit settingsChanged("settings");
    KConfigDialog::updateSettings();
}

void SettingsDialog::updateWidgets()
{

    KConfigDialog::updateWidgets();
}

void SettingsDialog::updateWidgetsDefault()
{

    KConfigDialog::updateWidgetsDefault();
}

bool SettingsDialog::hasChanged()
{
    if (m_changed) {
        return true;
    }
    return KConfigDialog::hasChanged();
}

bool SettingsDialog::isDefault()
{
    return false;
}
