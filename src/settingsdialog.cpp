#include "settingsdialog.h"
#include "settings.h"
#include "mainwindow.h"
#include "treewidget.h"

#include <QHeaderView>

void SettingsDialog::addNewMenu()
{
    auto item = new QTreeWidgetItem();
    item->setIcon(0, QIcon::fromTheme("application-menu"));
    item->setText(0, m_addMenuDialog->menuName->text());
    item->setToolTip(0, m_addMenuDialog->menuName->text());
    item->setData(0, Qt::UserRole, "menu");
    m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);
    m_changed = true;
    updateButtons();
}

SettingsDialog::SettingsDialog(QWidget *parent, const QString &name, KConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
    , m_config(KSharedConfig::openConfig("quickaccessrc"))
    , m_settings(new Settings(this))
    , m_changed(false)
{
    addPage(m_settings, i18n("Settings"));
    m_commandsTree = new TreeWidget();
    m_commandsTree->setColumnCount(2);
    m_commandsTree->headerItem()->setText(0, i18n("Name"));
    m_commandsTree->headerItem()->setText(1, i18n("Process"));
    m_commandsTree->headerItem()->setText(2, i18n("Args"));
    m_commandsTree->header()->setStretchLastSection(true);
    m_commandsTree->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_commandsTree->header()->resizeSection(0, 150);
    m_commandsTree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_commandsTree->header()->resizeSection(0, 150);
    m_commandsTree->setDragEnabled(true);
    m_commandsTree->setDragDropMode(QAbstractItemView::InternalMove);

    connect(m_commandsTree, &TreeWidget::drop, this, [=]() {
        m_changed = true;
        updateButtons();
    });
    qobject_cast<QVBoxLayout *>(m_settings->commandsGroupBox->layout())->insertWidget(0, m_commandsTree);

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

    populateTree();

    connect(m_settings->removeTreeItem, &QPushButton::clicked, this, [=]() {
        for (auto item : m_commandsTree->selectedItems()) {
            delete item;
        }
        m_changed = true;
        updateButtons();
    });

    // add action to the tree widget
    m_addActionDialog = new AddAction(nullptr);
    connect(m_settings->addActionButton, &QPushButton::clicked, m_addActionDialog, &AddAction::show);
    connect(m_addActionDialog, &AddAction::accepted, this, &SettingsDialog::addNewAction);
    connect(m_addActionDialog, &AddAction::finished, this, &SettingsDialog::clearAddActionFields);

    // add menu to the tree widget
    m_addMenuDialog = new AddMenu(nullptr);
    connect(m_settings->addMenu, &QPushButton::clicked, m_addMenuDialog, &AddMenu::show);
    connect(m_addMenuDialog, &AddMenu::accepted, this, &SettingsDialog::addNewMenu);
    connect(m_addMenuDialog, &AddMenu::finished, m_addMenuDialog->menuName, &QLineEdit::clear);
}

void SettingsDialog::addNewAction()
{
    auto item = new QTreeWidgetItem();
    item->setIcon(0, QIcon::fromTheme(m_addActionDialog->actionIcon->text()));
    item->setText(0, m_addActionDialog->actionName->text());
    item->setToolTip(0, m_addActionDialog->actionName->text());
    item->setText(1, m_addActionDialog->actionProcess->text());
    item->setToolTip(1, m_addActionDialog->actionProcess->text());
    item->setText(2, m_addActionDialog->actionArgs->text());
    item->setToolTip(2, m_addActionDialog->actionArgs->text());
    item->setData(0, Qt::UserRole, "action");
    m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);
    m_changed = true;
    updateButtons();
}

void SettingsDialog::clearAddActionFields()
{
    m_addActionDialog->actionProcess->clear();
    m_addActionDialog->actionArgs->clear();
    m_addActionDialog->actionName->clear();
    m_addActionDialog->actionIcon->clear();
}

void SettingsDialog::populateTree()
{
    auto commandsCount = m_config->group("Commands").readEntry("Count");
    for (int i = 0; i < commandsCount.toInt(); i++) {
        auto command = m_config->group(QString("Command_%1").arg(i));
        auto item = new QTreeWidgetItem();
        item->setIcon(0, QIcon::fromTheme(command.readEntry("Icon")));
        item->setText(0, command.readEntry("Name"));
        item->setToolTip(0, command.readEntry("Name"));
        item->setData(0, Qt::UserRole, command.readEntry("Type"));
        if (command.readEntry("Type") == "action") {
            item->setText(1, command.readEntry("Process"));
            item->setToolTip(1, command.readEntry("Process"));
            item->setText(2, command.readEntry("Args"));
            item->setToolTip(2, command.readEntry("Args"));
        }
        m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);

        int subCommands = command.readEntry("Count").toInt();
        if (subCommands > 0) {
            for (int j = 0; j < subCommands; ++j) {
                auto command = m_config->group(QString("Command_%1__Action_%2").arg(i).arg(j));
                auto childItem = new QTreeWidgetItem();
                childItem->setIcon(0, QIcon::fromTheme(command.readEntry("Icon")));
                childItem->setText(0, command.readEntry("Name"));
                childItem->setToolTip(0, command.readEntry("Name"));
                childItem->setText(1, command.readEntry("Process"));
                childItem->setToolTip(1, command.readEntry("Process"));
                childItem->setText(2, command.readEntry("Args"));
                childItem->setToolTip(2, command.readEntry("Args"));
                childItem->setData(0, Qt::UserRole, command.readEntry("Type"));
                item->addChild(childItem);
            }
        }
    }
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

void SettingsDialog::saveCommands()
{
    int commandsCount = m_commandsTree->model()->rowCount();
    // save the commands
    for (int i = 0; i < commandsCount; i++) {
        auto item = m_commandsTree->topLevelItem(i);
        QString name = item->text(0);
        QString iconName = item->icon(0).name();
        QString command = item->text(1);
        QString args = item->text(2);

        auto group = m_config->group(QString("Command_%1").arg(i));
        group.writeEntry("Name", name);
        group.writeEntry("Icon", iconName);
        group.writeEntry("Process", command);
        group.writeEntry("Args", args);
        group.writeEntry("Count", item->childCount());
        group.writeEntry("Type", item->data(0, Qt::UserRole));

        if (group.readEntry("Type") == "menu") {
            for (int j = 0; j < item->childCount(); ++j) {
                auto group = m_config->group(QString("Command_%1__Action_%2").arg(i).arg(j));
                group.writeEntry("Name", item->child(j)->text(0));
                group.writeEntry("Icon", item->child(j)->icon(0).name());
                group.writeEntry("Process", item->child(j)->text(1));
                group.writeEntry("Args", item->child(j)->text(2));
                group.writeEntry("Type", item->data(0, Qt::UserRole));
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
