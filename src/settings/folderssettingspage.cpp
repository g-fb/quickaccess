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

    auto const folders = QuickAccessSettings::paths();
    auto model = new QStandardItemModel(folders.count(), 2);
    int i {0};
    for (auto folder : folders) {
        QStandardItem *item = new QStandardItem();
        item->setData(folder, Qt::DisplayRole);
        item->setData(QIcon::fromTheme("folder"), Qt::DecorationRole);
        if (i == 2) {
            item->setData(QIcon::fromTheme("configure"), Qt::DecorationRole);
        }
        model->setItem(i, item);
        ++i;
    }

    auto foldersListView = new QListView(this);
    foldersListView->setModel(model);
    foldersListView->setEditTriggers(QAbstractItemView::NoEditTriggers);


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

    foldersLayout->addWidget(foldersListView);
    foldersLayout->addWidget(editWidget);
    foldersLayout->addWidget(buttonsWidget);
    foldersLayout->addSpacing(25);


    auto selectedIndex = [=]() {
        QItemSelectionModel *selection = foldersListView->selectionModel();
        const QModelIndexList selectedIndexes = selection->selectedIndexes();
        if (!selectedIndexes.isEmpty() && selectedIndexes[0].isValid()) {
            return selectedIndexes[0];
        } else {
            return QModelIndex();
        }
    };

    // Connections

    connect(foldersListView->selectionModel(), &QItemSelectionModel::selectionChanged,
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

            foldersListView->selectionModel()->select(index, QItemSelectionModel::Deselect);
            foldersListView->selectionModel()->select(aboveIndex, QItemSelectionModel::Select);

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

            foldersListView->selectionModel()->select(index, QItemSelectionModel::Deselect);
            foldersListView->selectionModel()->select(aboveIndex, QItemSelectionModel::Select);

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
        foldersListView->selectionModel()->select(newIndex, QItemSelectionModel::Select);
    });

    connect(model, &QStandardItemModel::itemChanged, this, &FoldersSettingsPage::changed);
}
