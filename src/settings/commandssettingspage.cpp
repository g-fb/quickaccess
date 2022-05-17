#include "commandssettingspage.h"

#include <KLocalizedString>

#include <KIconButton>
#include <QtWidgets>

#include "settings.h"
#include "treewidget.h"

CommandsSettingsPage::CommandsSettingsPage(QWidget *parent)
    : QWidget(parent)
{
    m_config = KSharedConfig::openConfig("quickaccessrc");

    createMenuDialog();
    createCommandDialog();

    auto commandsMainLayout = new QVBoxLayout(this);

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

    int commandsCount = m_config->group("General").readEntry("CommandsCount").toInt();
    for (int i = 0; i < commandsCount; i++) {
        auto groupName = QString("Command_%0").arg(i);
        auto group = m_config->group(groupName);
        auto item = createItem(group);
        m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);

        int subCommands = group.readEntry("Count").toInt();
        for (int j = 0; j < subCommands; ++j) {
            auto groupName = QString("Command_%0__Subcommand_%1").arg(i).arg(j);
            auto group = m_config->group(groupName);
            QTreeWidgetItem *childItem = createItem(group);
            item->addChild(childItem);
        }
    }

    auto commandsButtonsWidget = new QWidget(this);
    auto commandsButtonsLayout = new QHBoxLayout(commandsButtonsWidget);
    commandsButtonsLayout->setContentsMargins(0, 0, 0, 0);

    auto addMenuButton = new QPushButton(this);
    addMenuButton->setText(i18n("Add menu"));
    auto addCommandButton = new QPushButton(this);
    addCommandButton->setText(i18n("Add command"));

    commandsButtonsLayout->addWidget(addMenuButton);
    commandsButtonsLayout->addWidget(addCommandButton);
    commandsButtonsLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

    commandsMainLayout->addWidget(m_commandsTree);
    commandsMainLayout->addWidget(commandsButtonsWidget);
    commandsMainLayout->addSpacing(25);


    // Connections
    connect(m_commandsTree, &TreeWidget::drop,
            this, &CommandsSettingsPage::changed);

    connect(m_commandsTree->model(), &QAbstractItemModel::dataChanged,
            this, &CommandsSettingsPage::changed);

    connect(addMenuButton, &QPushButton::clicked, this, [=]() {
        m_dialogMode = QA::DialogNewMode;
        m_menuDialog->setWindowTitle(i18n("New Menu"));
        m_menuNameInput->clear();
        m_menuDialog->show();
    });

    connect(addCommandButton, &QPushButton::clicked, this, [=]() {
        m_dialogMode = QA::DialogNewMode;
        m_commandDialog->setWindowTitle(i18n("New Menu"));
        m_commandNameInput->clear();
        m_processNameInput->clear();
        m_argumentsInput->clear();
        m_iconSelectButton->setIcon(QIcon::fromTheme("unknown"));
        m_commandDialog->show();
    });

    connect(m_commandsTree, &QTreeWidget::customContextMenuRequested,
            this, &CommandsSettingsPage::contextMenu);

    connect(m_commandsTree, &QTreeWidget::doubleClicked, this, [=]() {
        auto item = m_commandsTree->currentItem();
        if (item->data(0, Qt::UserRole) == "command") {
            editCommand();
        }
    });
}

void CommandsSettingsPage::save()
{
    deleteCommands();

    int commandsCount = m_commandsTree->topLevelItemCount();
    // save the commands
    for (int i = 0; i < commandsCount; ++i) {
        auto item = m_commandsTree->topLevelItem(i);
        QString name = item->text(0);
        QString iconName = item->icon(0).name();
        if (iconName.isEmpty()) {
            iconName = item->data(0, Qt::UserRole + 100).toString();
        }
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
                QString iconName = item->child(j)->icon(0).name();
                if (iconName.isEmpty()) {
                    iconName = item->child(j)->data(0, Qt::UserRole + 100).toString();
                }
                group.writeEntry("Icon", iconName);
                group.writeEntry("Process", item->child(j)->text(1));
                group.writeEntry("Args", item->child(j)->text(2));
                group.writeEntry("Type", item->child(j)->data(0, Qt::UserRole));
            }
        }
    }
    auto group = m_config->group("General");
    group.writeEntry("CommandsCount", QString::number(commandsCount));
    m_config->sync();

}

void CommandsSettingsPage::deleteCommands()
{
    int commandsCount = m_config->group("General").readEntry("CommandsCount").toInt();
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

void CommandsSettingsPage::createMenuDialog()
{
    if (m_menuDialog == nullptr) {
        return;
    }
    m_menuDialog = new QDialog(this);
    m_menuDialog->setMinimumWidth(300);
    m_menuDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_menuDialogLayout = new QVBoxLayout(m_menuDialog);

    auto menuNameLabel = new QLabel(i18n("Menu name"), m_menuDialog);
    m_menuNameInput = new QLineEdit(m_menuDialog);

    m_menuDialogLayout->addWidget(menuNameLabel);
    m_menuDialogLayout->addWidget(m_menuNameInput);
    m_menuDialogLayout->addSpacing(25);
    m_menuDialogLayout->addWidget(m_menuDialogButtonBox);

    connect(m_menuDialogButtonBox, &QDialogButtonBox::accepted, m_menuDialog, &QDialog::accept);
    connect(m_menuDialogButtonBox, &QDialogButtonBox::rejected, m_menuDialog, &QDialog::reject);
    connect(m_menuDialogButtonBox, &QDialogButtonBox::accepted, this, [=]() {
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
        item->setText(0, m_menuNameInput->text());
        item->setToolTip(0, m_menuNameInput->text());
        item->setData(0, Qt::UserRole, "menu");
    });
}

void CommandsSettingsPage::createCommandDialog()
{
    if (m_commandDialog == nullptr) {
        return;
    }
    m_commandDialog = new QDialog(this);
    m_commandDialog->setMinimumWidth(400);
    m_commandDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_commandDialogLayout = new QVBoxLayout(m_commandDialog);

    auto commandNameLabel = new QLabel(i18n("Command name"), m_commandDialog);
    m_commandNameInput = new QLineEdit(m_commandDialog);
    auto processNameLabel = new QLabel(i18n("Process"), m_commandDialog);
    m_processNameInput = new QLineEdit(m_commandDialog);
    auto argumentsLabel = new QLabel(i18n("Arguments"), m_commandDialog);
    m_argumentsInput = new QLineEdit(m_commandDialog);
    m_iconSelectButton = new KIconButton(m_commandDialog);
    m_iconSelectButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_iconSelectButton->setIcon(QIcon::fromTheme("unknown"));
    m_iconSelectButton->setButtonIconSize(22);
    m_iconSelectButton->setText(i18n("Select icon"));

    m_commandDialogLayout->addWidget(commandNameLabel);
    m_commandDialogLayout->addWidget(m_commandNameInput);
    m_commandDialogLayout->addWidget(processNameLabel);
    m_commandDialogLayout->addWidget(m_processNameInput);
    m_commandDialogLayout->addWidget(argumentsLabel);
    m_commandDialogLayout->addWidget(m_argumentsInput);
    auto w = new QWidget();
    auto l = new QHBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_iconSelectButton);
    l->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    m_commandDialogLayout->addWidget(w);
    m_commandDialogLayout->addSpacing(25);
    m_commandDialogLayout->addWidget(m_commandDialogButtonBox);

    connect(m_commandDialogButtonBox, &QDialogButtonBox::accepted, m_commandDialog, &QDialog::accept);
    connect(m_commandDialogButtonBox, &QDialogButtonBox::rejected, m_commandDialog, &QDialog::reject);
    connect(m_commandDialogButtonBox, &QDialogButtonBox::accepted, this, [=]() {
        QTreeWidgetItem *item;
        if (m_dialogMode == QA::DialogNewMode) {
            // create new item
            item = new QTreeWidgetItem();
            m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), item);
        } else {
            // edit the current item
            item = m_commandsTree->currentItem();
        }

        item->setIcon(0, QIcon::fromTheme(m_iconSelectButton->icon()));
        item->setData(0, Qt::UserRole + 100, m_iconSelectButton->icon());
        item->setText(0, m_commandNameInput->text());
        item->setToolTip(0, m_commandNameInput->text());
        item->setText(1, m_processNameInput->text());
        item->setToolTip(1, m_processNameInput->text());
        item->setText(2, m_argumentsInput->text());
        item->setToolTip(2, m_argumentsInput->text());
        item->setData(0, Qt::UserRole, "command");
    });
}

void CommandsSettingsPage::cloneCommand()
{
    auto item = m_commandsTree->currentItem();
    auto clone = item->clone();
    if (item->parent() && item->parent()->data(0, Qt::UserRole) == "menu") {
        item->parent()->addChild(clone);
    } else {
        m_commandsTree->insertTopLevelItem(m_commandsTree->topLevelItemCount(), clone);
    }
}

void CommandsSettingsPage::editCommand()
{
    m_dialogMode = QA::DialogEditMode;
    auto item = m_commandsTree->currentItem();
    if (item->data(0, Qt::UserRole) == "menu") {
        m_menuDialog->setWindowTitle(i18n("Edit Menu"));
        m_menuNameInput->setText(item->text(0));
        m_menuDialog->show();
    } else {
        m_commandDialog->setWindowTitle(i18n("Edit Command"));
        m_commandNameInput->setText(item->text(0));
        m_processNameInput->setText(item->text(1));
        m_argumentsInput->setText(item->text(2));
        m_iconSelectButton->setIcon(item->icon(0).name());
        m_commandDialog->show();
    }
}

void CommandsSettingsPage::contextMenu()
{
    auto menu = new QMenu();

    auto command = new QAction(nullptr);
    command->setText(i18n("Edit"));
    command->setIcon(QIcon::fromTheme("edit-entry"));
    menu->addAction(command);
    connect(command, &QAction::triggered, this, &CommandsSettingsPage::editCommand);

    command = new QAction(nullptr);
    command->setText(i18n("Clone"));
    command->setIcon(QIcon::fromTheme("edit-copy"));
    menu->addAction(command);
    connect(command, &QAction::triggered, this, &CommandsSettingsPage::cloneCommand);

    menu->addSeparator();

    command = new QAction(nullptr);
    command->setText(i18n("Remove"));
    command->setIcon(QIcon::fromTheme("edit-delete-remove"));
    menu->addAction(command);
    connect(command, &QAction::triggered, this, [=]() {
        delete m_commandsTree->currentItem();
    });

    menu->exec(QCursor::pos());
}

QTreeWidgetItem *CommandsSettingsPage::createItem(KConfigGroup group)
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
