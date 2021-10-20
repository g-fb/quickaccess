#include "folderssettingspage.h"

#include <KIconDialog>
#include <KLocalizedString>

#include <QtWidgets>

#include "settings.h"

FoldersSettingsPage::FoldersSettingsPage(QWidget *parent)
    : QWidget(parent)
{
    auto iconDialog = new KIconDialog(this);
    auto foldersLayout = new QVBoxLayout(this);

    m_config = KSharedConfig::openConfig("quickaccessrc");

    auto group = m_config->group(QStringLiteral("General"));
    int foldersCount = group.readEntry("FoldersCount").toInt();

    auto model = new QStandardItemModel(foldersCount, 2);

    for (int i = 0; i < foldersCount; ++i) {
        auto group = m_config->group(QStringLiteral("Folder_%1").arg(i));
        auto path = group.readEntry(QStringLiteral("Path"));
        auto iconName = group.readEntry(QStringLiteral("Icon"));

        QStandardItem *item = new QStandardItem();
        item->setData(path, Qt::DisplayRole);
        item->setData(QIcon::fromTheme(iconName), Qt::DecorationRole);
        model->setItem(i, item);
    }

    m_foldersListView = new QListView(this);
    m_foldersListView->setModel(model);
    m_foldersListView->setEditTriggers(QAbstractItemView::NoEditTriggers);


    auto editWidget = new QWidget(this);
    editWidget->setEnabled(false);
    auto editLayout = new QHBoxLayout(editWidget);
    editLayout->setContentsMargins(0, 0, 0, 0);

    auto iconPreview = new QPushButton(this);
    iconPreview->setIcon(QIcon::fromTheme("unknown"));

    auto folderEdit = new QLineEdit(this);
    folderEdit->setPlaceholderText(i18n("Folder"));
    folderEdit->setToolTip(folderEdit->placeholderText());
    folderEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    editLayout->addWidget(iconPreview);
    editLayout->addWidget(folderEdit);


    auto buttonsWidget = new QWidget(this);
    auto buttonsLayout = new QHBoxLayout(buttonsWidget);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    auto moveUpButton = new QPushButton(this);
    moveUpButton->setText(i18n("Move Up"));
    moveUpButton->setIcon(QIcon::fromTheme("arrow-up"));
    buttonsLayout->addWidget(moveUpButton);

    auto moveDownButton = new QPushButton(this);
    moveDownButton->setText(i18n("Move Down"));
    moveDownButton->setIcon(QIcon::fromTheme("arrow-down"));
    buttonsLayout->addWidget(moveDownButton);

    auto addFolderButton = new QPushButton(this);
    addFolderButton->setText(i18n("Add"));
    addFolderButton->setIcon(QIcon::fromTheme("list-add"));
    buttonsLayout->addWidget(addFolderButton);

    auto spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    buttonsLayout->addSpacerItem(spacer);

    auto deleteFolderButton = new QPushButton(this);
    deleteFolderButton->setText(i18n("Delete"));
    deleteFolderButton->setIcon(QIcon::fromTheme("edit-delete"));
    buttonsLayout->addWidget(deleteFolderButton);

    foldersLayout->addWidget(m_foldersListView);
    foldersLayout->addWidget(editWidget);
    foldersLayout->addWidget(buttonsWidget);
    foldersLayout->addSpacing(25);


    auto selectedIndex = [=]() {
        QItemSelectionModel *selection = m_foldersListView->selectionModel();
        const QModelIndexList selectedIndexes = selection->selectedIndexes();
        if (!selectedIndexes.isEmpty() && selectedIndexes[0].isValid()) {
            return selectedIndexes[0];
        } else {
            return QModelIndex();
        }
    };

    // Connections

    connect(m_foldersListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [=](const QItemSelection &, const QItemSelection &) {
        auto index = selectedIndex();
        auto iconName = index.data(Qt::DecorationRole).value<QIcon>().name();
        editWidget->setEnabled(true);
        folderEdit->setText(index.data().toString());
        iconPreview->setIcon(QIcon::fromTheme(iconName));
    });

    connect(iconPreview, &QPushButton::clicked, this, [=]() {
        auto index = selectedIndex();
        auto item = model->item(index.row(), index.column());
        auto oldIconName = item->data(Qt::DecorationRole).value<QIcon>().name();
        auto newIconName = iconDialog->getIcon();
        auto iconName = newIconName.isEmpty() ? oldIconName : newIconName;
        item->setData(QIcon::fromTheme(iconName), Qt::DecorationRole);
        iconPreview->setIcon(QIcon::fromTheme(iconName));
    });

    connect(folderEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        auto index = selectedIndex();
        model->setData(index, text, Qt::DisplayRole);
    });

    connect(moveUpButton, &QPushButton::clicked, this, [=]() {
        auto index = selectedIndex();
        if (index.isValid() && index.row() > 0) {
            auto aboveIndex = model->index(index.row() - 1, 0);

            auto tmpFolder = model->data(aboveIndex, Qt::DisplayRole).toString();
            auto tmpIcon = model->data(aboveIndex, Qt::DecorationRole).value<QIcon>();

            model->setData(aboveIndex, model->data(index, Qt::DisplayRole), Qt::DisplayRole);
            model->setData(aboveIndex, model->data(index, Qt::DecorationRole), Qt::DecorationRole);

            model->setData(index, tmpFolder, Qt::DisplayRole);
            model->setData(index, tmpIcon, Qt::DecorationRole);

            m_foldersListView->selectionModel()->select(index, QItemSelectionModel::Deselect);
            m_foldersListView->selectionModel()->select(aboveIndex, QItemSelectionModel::Select);

            Q_EMIT changed();
        }
    });

    connect(moveDownButton, &QPushButton::clicked, this, [=]() {
        auto index = selectedIndex();
        if (index.isValid() && index.row() < model->rowCount()-1) {
            auto aboveIndex = model->index(index.row() + 1, 0);

            auto tmpFolder = model->data(aboveIndex, Qt::DisplayRole).toString();
            auto tmpIcon = model->data(aboveIndex, Qt::DecorationRole).value<QIcon>();

            model->setData(aboveIndex, model->data(index, Qt::DisplayRole), Qt::DisplayRole);
            model->setData(aboveIndex, model->data(index, Qt::DecorationRole), Qt::DecorationRole);

            model->setData(index, tmpFolder, Qt::DisplayRole);
            model->setData(index, tmpIcon, Qt::DecorationRole);

            m_foldersListView->selectionModel()->select(index, QItemSelectionModel::Deselect);
            m_foldersListView->selectionModel()->select(aboveIndex, QItemSelectionModel::Select);

            Q_EMIT changed();
        }
    });

    connect(addFolderButton, &QPushButton::clicked, this, [=]() {
        QString path = QFileDialog::getExistingDirectory(this, i18n("Select a folder"), QDir::homePath());
        if (!path.isEmpty()) {
            model->insertRow(model->rowCount(), new QStandardItem(QIcon::fromTheme("folder"), path));

            Q_EMIT changed();
        }
    });

    connect(deleteFolderButton, &QPushButton::clicked, this, [=]() {
        auto index = selectedIndex();
        model->removeRows(index.row(), 1);

        Q_EMIT changed();
    });

    connect(model, &QStandardItemModel::rowsInserted, this, [=](const QModelIndex &, int, int last) {
        QModelIndex newIndex = model->index(last, 0);
        m_foldersListView->selectionModel()->select(newIndex, QItemSelectionModel::Select);
    });

    connect(model, &QStandardItemModel::itemChanged, this, &FoldersSettingsPage::changed);
}

void FoldersSettingsPage::save()
{
    deleteFolders();

    auto model = m_foldersListView->model();
    int foldersCount = model->rowCount();
    // save the folders
    for (int i = 0; i < foldersCount; ++i) {
        auto modelIndex = model->index(i, 0);
        QString path = modelIndex.data(Qt::DisplayRole).toString();
        QString iconName = modelIndex.data(Qt::DecorationRole).value<QIcon>().name();

        auto group = m_config->group(QString("Folder_%1").arg(i));
        group.writeEntry(QStringLiteral("Path"), path);
        group.writeEntry(QStringLiteral("Icon"), iconName);
        m_config->sync();
    }
    auto group = m_config->group("General");
    group.writeEntry("FoldersCount", QString::number(foldersCount));
    m_config->sync();
}

void FoldersSettingsPage::deleteFolders()
{
    int foldersCount = m_config->group("General").readEntry("FoldersCount").toInt();
    // delete all folders
    for (int i = 0; i < foldersCount; ++i) {
        auto group = m_config->group(QString("Folder_%0").arg(i));
        m_config->deleteGroup(group.name());
    }
    m_config->deleteGroup("Folders");
    m_config->sync();
}
