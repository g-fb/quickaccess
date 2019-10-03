#include "settingsdialog.h"
#include "settings.h"
#include "mainwindow.h"
#include "treewidget.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>

SettingsDialog::SettingsDialog(QWidget *parent, const QString &name, KConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
    , m_config(KSharedConfig::openConfig("quickaccessrc"))
    , m_settings(new Settings(this))
    , m_changed(false)
    , m_dialogMode(QA::DialogNewMode)
{
    addPage(m_settings, i18n("Settings"));
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
    connect(m_commandsTree, &TreeWidget::drop, this, [=]() {
        m_changed = true;
        updateButtons();
    });
    // insert the commands tree widget in the dialog's group box
    auto treeWidgetLayout = qobject_cast<QVBoxLayout *>(m_settings->commandsGroupBox->layout());
    treeWidgetLayout->insertWidget(0, m_commandsTree);
    populateTree();

    m_settings->submenuEntriesCountInfo->setText(
                i18n("Use %1 to show all or %2 to show none").arg("-1").arg(0));

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

    // add action to the tree widget
    m_addActionDialog = new AddAction(nullptr);
    connect(m_settings->addActionButton, &QPushButton::clicked, this, [=]() {
        m_dialogMode = QA::DialogNewMode;
        m_addActionDialog->setWindowTitle(i18n("New Action"));
        m_addActionDialog->actionProcess->clear();
        m_addActionDialog->actionArgs->clear();
        m_addActionDialog->actionName->clear();
        m_addActionDialog->actionIcon->clear();
        m_addActionDialog->show();
    });
    connect(m_addActionDialog, &AddAction::accepted, this, &SettingsDialog::manageAction);

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

void SettingsDialog::manageAction()
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
    item->setIcon(0, QIcon::fromTheme(m_addActionDialog->actionIcon->text()));
    item->setText(0, m_addActionDialog->actionName->text());
    item->setToolTip(0, m_addActionDialog->actionName->text());
    item->setText(1, m_addActionDialog->actionProcess->text());
    item->setToolTip(1, m_addActionDialog->actionProcess->text());
    item->setText(2, m_addActionDialog->actionArgs->text());
    item->setToolTip(2, m_addActionDialog->actionArgs->text());
    item->setData(0, QA::TypeRole, "action");

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
    item->setData(0, QA::TypeRole, "menu");

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
    if (type == "action") {
        item->setText(1, group.readEntry("Process"));
        item->setToolTip(1, group.readEntry("Process"));
        item->setText(2, group.readEntry("Args"));
        item->setToolTip(2, group.readEntry("Args"));
    }
    item->setData(0, QA::TypeRole, type);

    return item;
}

void SettingsDialog::contextMenu()
{
    auto menu = new QMenu();

    auto action = new QAction();
    action->setText(i18n("Edit"));
    action->setIcon(QIcon::fromTheme("edit-entry"));
    menu->addAction(action);
    connect(action, &QAction::triggered, this, &SettingsDialog::editCommand);

    action = new QAction();
    action->setText(i18n("Clone"));
    action->setIcon(QIcon::fromTheme("edit-copy"));
    menu->addAction(action);
    connect(action, &QAction::triggered, this, &SettingsDialog::cloneCommand);

    menu->addSeparator();

    action = new QAction();
    action->setText(i18n("Remove"));
    action->setIcon(QIcon::fromTheme("edit-delete-remove"));
    menu->addAction(action);
    connect(action, &QAction::triggered, this, [=]() {
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
        auto group = m_config->group(QString("Command_%1").arg(i));
        if (group.readEntry("Type") == "menu") {
            for (int j = 0; j < group.readEntry("Count"); ++j) {
                auto group = m_config->group(QString("Command_%1__Action_%2").arg(i).arg(j));
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
    if (item->data(0, QA::TypeRole) == "menu") {
        m_addMenuDialog->setWindowTitle(i18n("Edit Menu"));
        m_addMenuDialog->menuName->setText(item->text(0));
        m_addMenuDialog->show();
    } else {
        m_addActionDialog->setWindowTitle(i18n("Edit Action"));
        m_addActionDialog->actionProcess->setText(item->text(1));
        m_addActionDialog->actionArgs->setText(item->text(2));
        m_addActionDialog->actionName->setText(item->text(0));
        m_addActionDialog->actionIcon->setText(item->icon(0).name());
        m_addActionDialog->show();
    }
}

void SettingsDialog::cloneCommand()
{
    auto item = m_commandsTree->currentItem();
    auto clone = item->clone();
    if (item->parent() && item->parent()->data(0, QA::TypeRole) == "menu") {
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
        auto groupName = QString("Command_%1").arg(i);
        auto group = m_config->group(groupName);
        auto item = createItemFromConfig(group);
        m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);

        int subCommands = group.readEntry("Count").toInt();
        for (int j = 0; j < subCommands; ++j) {
            auto groupName = QString("Command_%1__Action_%2").arg(i).arg(j);
            auto group = m_config->group(groupName);
            auto childItem = createItemFromConfig(group);
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

        auto group = m_config->group(QString("Command_%1").arg(i));
        group.writeEntry("Name", name);
        group.writeEntry("Icon", iconName);
        if (item->data(0, QA::TypeRole) == "action") {
            group.writeEntry("Process", process);
            group.writeEntry("Args", args);
        }
        group.writeEntry("Count", item->childCount());
        group.writeEntry("Type", item->data(0, QA::TypeRole));
        m_config->sync();

        if (item->data(0, QA::TypeRole) == "menu") {
            for (int j = 0; j < item->childCount(); ++j) {
                auto group = m_config->group(QString("Command_%1__Action_%2").arg(i).arg(j));
                group.writeEntry("Name", item->child(j)->text(0));
                group.writeEntry("Icon", item->child(j)->icon(0).name());
                group.writeEntry("Process", item->child(j)->text(1));
                group.writeEntry("Args", item->child(j)->text(2));
                group.writeEntry("Type", item->child(j)->data(0, QA::TypeRole));
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
