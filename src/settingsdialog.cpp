#include "settingsdialog.h"
#include "settings.h"
#include "mainwindow.h"
#include "treewidget.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>

SettingsDialog::SettingsDialog(Settings *settings, QWidget *parent, const QString &name, KConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
    , m_config(KSharedConfig::openConfig("quickaccessrc"))
    , m_settings(settings)
    , m_changed(false)
    , m_dialogMode(QA::DialogNewMode)
{
    addPage(m_settings, i18n("Settings"));
    setHelp(QStringLiteral(), QStringLiteral("com.georgefb.quickaccess"));
    // setup the commands tree
    m_commandsTree = new TreeWidget();
    m_commandsTree->setColumnCount(2);
    m_commandsTree->header()->setStretchLastSection(true);
    m_commandsTree->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_commandsTree->header()->resizeSection(0, 150);
    m_commandsTree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_commandsTree->header()->resizeSection(0, 150);
    m_commandsTree->headerItem()->setText(0, i18n("Name"));
    m_commandsTree->headerItem()->setText(1, i18n("Process"));
    m_commandsTree->headerItem()->setText(2, i18n("Args"));
    m_commandsTree->setDragEnabled(true);
    m_commandsTree->setDragDropMode(QAbstractItemView::InternalMove);
    m_commandsTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_commandsTree, &QTreeWidget::customContextMenuRequested,
            this, &SettingsDialog::contextMenu);
    connect(m_commandsTree, &QTreeWidget::doubleClicked, this, [=]() {
        auto item = m_commandsTree->currentItem();
        if (item->data(0, Qt::UserRole) == "command") {
            editCommand();
        }
    });
    connect(m_commandsTree, &TreeWidget::drop, this, [=]() {
        m_changed = true;
        updateButtons();
    });
    // insert the commands tree widget in the dialog's group box
    auto treeWidgetLayout = qobject_cast<QVBoxLayout *>(m_settings->commandsTab->layout());
    treeWidgetLayout->insertWidget(0, m_commandsTree);
    populateTree();

    m_settings->submenuEntriesCountInfo->setText(
                i18n("Use %1 to show all or %2 to show none", QStringLiteral("-1"), QStringLiteral("0")));

    // add button to open file dialog to select a folder
    // and add it to the folders list
    auto addFolderButton = new QPushButton(i18n("Select and Add Folder"));
    addFolderButton->setIcon(QIcon::fromTheme("folder-add"));
    connect(addFolderButton, &QPushButton::clicked, this, [=]() {
        QString path = QFileDialog::getExistingDirectory(
                    this, i18n("Select a folder"), QDir::homePath());
        if (!path.isEmpty()) {
            m_settings->kcfg_paths->insertItem(path, m_settings->kcfg_paths->count());
            emit m_settings->kcfg_paths->changed();
        }
    });
    auto widget = new QWidget();
    auto hLayout = new QHBoxLayout(widget);
    hLayout->setMargin(0);
    hLayout->addWidget(addFolderButton);
    hLayout->addStretch(1);
    // add widget to the keditlistwidget's layout
    m_settings->kcfg_paths->layout()->addWidget(widget);

    // add command to the tree widget
    m_addCommandDialog = new AddCommand(nullptr);
    connect(m_settings->addCommandButton, &QPushButton::clicked, this, [=]() {
        m_dialogMode = QA::DialogNewMode;
        m_addCommandDialog->setWindowTitle(i18n("New Command"));
        m_addCommandDialog->commandProcess->clear();
        m_addCommandDialog->commandArgs->clear();
        m_addCommandDialog->commandName->clear();
        m_addCommandDialog->commandIcon->clear();
        m_addCommandDialog->show();
    });
    connect(m_addCommandDialog, &AddCommand::accepted, this, &SettingsDialog::manageCommand);

    // add menu to the tree widget
    m_addMenuDialog = new AddMenu(nullptr);
    connect(m_settings->addMenu, &QPushButton::clicked, this, [=]() {
        m_dialogMode = QA::DialogNewMode;
        m_addMenuDialog->setWindowTitle(i18n("New Menu"));
        m_addMenuDialog->menuName->clear();
        m_addMenuDialog->show();
    });
    connect(m_addMenuDialog, &AddMenu::accepted, this, &SettingsDialog::manageMenu);
}

void SettingsDialog::manageCommand()
{
    QTreeWidgetItem *item;
    if (m_dialogMode == QA::DialogNewMode) {
        // create new item
        item = new QTreeWidgetItem();
        m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);
    } else {
        // edit the current item
        item = m_commandsTree->currentItem();
    }
    item->setIcon(0, QIcon::fromTheme(m_addCommandDialog->commandIcon->text()));
    item->setText(0, m_addCommandDialog->commandName->text());
    item->setToolTip(0, m_addCommandDialog->commandName->text());
    item->setText(1, m_addCommandDialog->commandProcess->text());
    item->setToolTip(1, m_addCommandDialog->commandProcess->text());
    item->setText(2, m_addCommandDialog->commandArgs->text());
    item->setToolTip(2, m_addCommandDialog->commandArgs->text());
    item->setData(0, Qt::UserRole, "command");

    m_changed = true;
    updateButtons();
}

void SettingsDialog::manageMenu()
{
    QTreeWidgetItem *item;
    if (m_dialogMode == QA::DialogNewMode) {
        // create new item
        item = new QTreeWidgetItem();
        m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);
    } else {
        // edit the current item
        item = m_commandsTree->currentItem();
    }
    item->setIcon(0, QIcon::fromTheme("application-menu"));
    item->setText(0, m_addMenuDialog->menuName->text());
    item->setToolTip(0, m_addMenuDialog->menuName->text());
    item->setData(0, Qt::UserRole, "menu");

    m_changed = true;
    updateButtons();
}

QTreeWidgetItem *SettingsDialog::createItemFromConfig(KConfigGroup group)
{
    auto type = group.readEntry("Type") ;
    auto item = new QTreeWidgetItem();
    item->setIcon(0, QIcon::fromTheme(group.readEntry("Icon")));
    item->setText(0, group.readEntry("Name"));
    item->setToolTip(0, group.readEntry("Name"));
    if (type == "command") {
        item->setText(1, group.readEntry("Process"));
        item->setToolTip(1, group.readEntry("Process"));
        item->setText(2, group.readEntry("Args"));
        item->setToolTip(2, group.readEntry("Args"));
    }
    item->setData(0, Qt::UserRole, type);

    return item;
}

void SettingsDialog::contextMenu()
{
    auto menu = new QMenu();

    auto command = new QAction(nullptr);
    command->setText(i18n("Edit"));
    command->setIcon(QIcon::fromTheme("edit-entry"));
    menu->addAction(command);
    connect(command, &QAction::triggered, this, &SettingsDialog::editCommand);

    command = new QAction(nullptr);
    command->setText(i18n("Clone"));
    command->setIcon(QIcon::fromTheme("edit-copy"));
    menu->addAction(command);
    connect(command, &QAction::triggered, this, &SettingsDialog::cloneCommand);

    menu->addSeparator();

    command = new QAction(nullptr);
    command->setText(i18n("Remove"));
    command->setIcon(QIcon::fromTheme("edit-delete-remove"));
    menu->addAction(command);
    connect(command, &QAction::triggered, this, [=]() {
        delete m_commandsTree->currentItem();
        m_changed = true;
        updateButtons();
    });

    menu->exec(QCursor::pos());
}

void SettingsDialog::deleteCommands()
{
    int commandsCount = m_config->group("Commands").readEntry("Count").toInt();
    // delete all commands
    for (int i = 0; i < commandsCount; ++i) {
        auto group = m_config->group(QString("Command_%0").arg(i));
        if (group.readEntry("Type") == "menu") {
            for (int j = 0; j < group.readEntry("Count").toInt(); ++j) {
                auto group = m_config->group(QString("Command_%0__Subcommand_%1").arg(i).arg(j));
                m_config->deleteGroup(group.name());
            }
        }
        m_config->deleteGroup(group.name());
    }
    m_config->deleteGroup("Commands");
    m_config->sync();
}

void SettingsDialog::editCommand()
{
    m_dialogMode = QA::DialogEditMode;
    auto item = m_commandsTree->currentItem();
    if (item->data(0, Qt::UserRole) == "menu") {
        m_addMenuDialog->setWindowTitle(i18n("Edit Menu"));
        m_addMenuDialog->menuName->setText(item->text(0));
        m_addMenuDialog->show();
    } else {
        m_addCommandDialog->setWindowTitle(i18n("Edit Command"));
        m_addCommandDialog->commandProcess->setText(item->text(1));
        m_addCommandDialog->commandArgs->setText(item->text(2));
        m_addCommandDialog->commandName->setText(item->text(0));
        m_addCommandDialog->commandIcon->setText(item->icon(0).name());
        m_addCommandDialog->show();
    }
}

void SettingsDialog::cloneCommand()
{
    auto item = m_commandsTree->currentItem();
    auto clone = item->clone();
    if (item->parent() && item->parent()->data(0, Qt::UserRole) == "menu") {
        item->parent()->addChild(clone);
    } else {
        m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(),clone);
    }
    m_changed = true;
    updateButtons();
}

void SettingsDialog::populateTree()
{
    int commandsCount = m_config->group("Commands").readEntry("Count").toInt();
    for (int i = 0; i < commandsCount; i++) {
        auto groupName = QString("Command_%0").arg(i);
        auto group = m_config->group(groupName);
        auto item = createItemFromConfig(group);
        m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);

        int subCommands = group.readEntry("Count").toInt();
        for (int j = 0; j < subCommands; ++j) {
            auto groupName = QString("Command_%0__Subcommand_%1").arg(i).arg(j);
            auto group = m_config->group(groupName);
            QTreeWidgetItem *childItem = createItemFromConfig(group);
            item->addChild(childItem);
        }
    }
}

void SettingsDialog::saveCommands()
{
    int commandsCount = m_commandsTree->topLevelItemCount();
    // save the commands
    for (int i = 0; i < commandsCount; ++i) {
        auto item = m_commandsTree->topLevelItem(i);
        QString name = item->text(0);
        QString iconName = item->icon(0).name();
        QString process = item->text(1);
        QString args = item->text(2);

        auto group = m_config->group(QString("Command_%0").arg(i));
        group.writeEntry("Name", name);
        group.writeEntry("Icon", iconName);
        if (item->data(0, Qt::UserRole) == "command") {
            group.writeEntry("Process", process);
            group.writeEntry("Args", args);
        }
        group.writeEntry("Count", item->childCount());
        group.writeEntry("Type", item->data(0, Qt::UserRole));
        m_config->sync();

        if (item->data(0, Qt::UserRole) == "menu") {
            for (int j = 0; j < item->childCount(); ++j) {
                auto group = m_config->group(QString("Command_%0__Subcommand_%1").arg(i).arg(j));
                group.writeEntry("Name", item->child(j)->text(0));
                group.writeEntry("Icon", item->child(j)->icon(0).name());
                group.writeEntry("Process", item->child(j)->text(1));
                group.writeEntry("Args", item->child(j)->text(2));
                group.writeEntry("Type", item->child(j)->data(0, Qt::UserRole));
            }
        }
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
    m_commandsTree->clear();
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
