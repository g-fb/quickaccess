#include "folderssettingspage.h"

#include <KIconDialog>
#include <KLocalizedString>
#include <QClipboard>

#include <QtWidgets>

#include "settings.h"

FoldersSettingsPage::FoldersSettingsPage(QWidget *parent)
    : QWidget(parent)
{
    auto iconDialog = new KIconDialog(this);
    auto foldersLayout = new QVBoxLayout(this);

    m_config = KSharedConfig::openConfig(u"quickaccessrc"_qs);

    auto group = m_config->group(QStringLiteral("General"));
    int foldersCount = group.readEntry("FoldersCount").toInt();

    m_foldersListView = new QListWidget(this);
    m_foldersListView->setDragDropMode(QAbstractItemView::InternalMove);
    m_foldersListView->setDefaultDropAction(Qt::MoveAction);
    m_foldersListView->setDragEnabled(true);
    m_foldersListView->setAcceptDrops(true);
    m_foldersListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_foldersListView->setSelectionMode(QAbstractItemView::SingleSelection);

    for (int i = 0; i < foldersCount; ++i) {
        auto group = m_config->group(QStringLiteral("Folder_%1").arg(i));
        auto path = group.readEntry(QStringLiteral("Path"));
        auto iconName = group.readEntry(QStringLiteral("Icon"));
        auto type = group.readEntry(QStringLiteral("Type"));

        QListWidgetItem *item = new QListWidgetItem();
        item->setText(path);
        item->setIcon(QIcon::fromTheme(iconName));
        item->setToolTip(i18n("Double click to set icon"));
        if (type == QStringLiteral("separator")) {
            item->setData(Qt::UserRole, type);
        } else {
            item->setData(Qt::UserRole, QString());
        }
        m_foldersListView->addItem(item);
    }

    auto editWidget = new QWidget(this);
    editWidget->setEnabled(false);
    auto editLayout = new QHBoxLayout(editWidget);
    editLayout->setContentsMargins(0, 0, 0, 0);

    auto iconPreview = new QPushButton(this);
    iconPreview->setIcon(QIcon::fromTheme(u"unknown"_qs));

    auto folderEdit = new QLineEdit(this);
    folderEdit->setPlaceholderText(i18n("Folder"));
    folderEdit->setToolTip(folderEdit->placeholderText());
    folderEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(m_foldersListView, &QListWidget::itemClicked, this, [=](){
        folderEdit->setFocus(Qt::MouseFocusReason);
    });
    connect(m_foldersListView, &QListWidget::itemDoubleClicked, this, [=](){
        iconPreview->click();
    });


    editLayout->addWidget(iconPreview);
    editLayout->addWidget(folderEdit);


    auto buttonsWidget = new QWidget(this);
    auto buttonsLayout = new QHBoxLayout(buttonsWidget);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    auto moveUpButton = new QPushButton(this);
    moveUpButton->setText(i18n("Move Up"));
    moveUpButton->setIcon(QIcon::fromTheme(u"arrow-up"_qs));
    buttonsLayout->addWidget(moveUpButton);

    auto moveDownButton = new QPushButton(this);
    moveDownButton->setText(i18n("Move Down"));
    moveDownButton->setIcon(QIcon::fromTheme(u"arrow-down"_qs));
    buttonsLayout->addWidget(moveDownButton);

    QClipboard *clipboard = QGuiApplication::clipboard();
    auto addFolderButton = new QToolButton(this);
    addFolderButton->setText(i18n("Add"));
    addFolderButton->setIcon(QIcon::fromTheme(u"list-add"_qs));
    addFolderButton->setPopupMode(QToolButton::MenuButtonPopup);
    addFolderButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    auto addFolderButtonMenu = new QMenu(addFolderButton);
    addFolderButtonMenu->setToolTip(clipboard->text());
    connect(clipboard, &QClipboard::changed, this, [=](QClipboard::Mode mode) {
        if (mode == QClipboard::Clipboard) {
            addFolderButtonMenu->setToolTip(clipboard->text());
        }
    });

    auto *separatorActioon = new QAction(addFolderButtonMenu);
    separatorActioon->setText(i18n("Add separator"));
    connect(separatorActioon, &QAction::triggered, this, [=]() {
        auto *item = new QListWidgetItem(i18n("--- separator ---"));
        item->setData(Qt::UserRole, QStringLiteral("separator"));
        m_foldersListView->addItem(item);
        m_foldersListView->setCurrentItem(item);
        Q_EMIT changed();
    });
    addFolderButtonMenu->addAction(separatorActioon);

    auto clipboardAction = new QAction(addFolderButtonMenu);
    clipboardAction->setText(i18n("Add from clipboard"));
    connect(clipboardAction, &QAction::triggered, this, [=]() {
        QFileInfo fileInfo(clipboard->text());
        if (fileInfo.exists()) {
            auto *item = new QListWidgetItem(QIcon::fromTheme(u"folder"_qs), clipboard->text());
            item->setToolTip(i18n("Double click to set icon"));
            m_foldersListView->addItem(item);
            m_foldersListView->setCurrentItem(item);
            Q_EMIT changed();
        } else {
            QDialog dialog;
            dialog.setWindowTitle(i18n("Clipboard text is not a valid path"));
            dialog.setMinimumWidth(600);

            auto widget = new QWidget(&dialog);
            auto vLayout = new QVBoxLayout(widget);
            widget->setLayout(vLayout);
            auto label = new QLabel(i18n("Clipboard text:"), widget);
            auto infoLabel = new QTextEdit(clipboard->text(), widget);
            auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
            connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            vLayout->addWidget(label);
            vLayout->addWidget(infoLabel);
            vLayout->addWidget(buttonBox);
            dialog.setLayout(vLayout);
            dialog.exec();
        }
    });
    addFolderButtonMenu->addAction(clipboardAction);
    addFolderButton->setMenu(addFolderButtonMenu);
    buttonsLayout->addWidget(addFolderButton);

    auto spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    buttonsLayout->addSpacerItem(spacer);

    auto deleteFolderButton = new QPushButton(this);
    deleteFolderButton->setText(i18n("Delete"));
    deleteFolderButton->setIcon(QIcon::fromTheme(u"edit-delete"_qs));
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

    connect(m_foldersListView, &QListWidget::itemSelectionChanged, this, [=]() {
        if (m_foldersListView->currentRow() > 0) {
            moveUpButton->setEnabled(true);
        } else {
            moveUpButton->setEnabled(false);
        }

        if (m_foldersListView->currentRow() < m_foldersListView->count() - 1) {
            moveDownButton->setEnabled(true);
        } else {
            moveDownButton->setEnabled(false);
        }
    });

    connect(m_foldersListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [=](const QItemSelection &, const QItemSelection &) {
        auto index = selectedIndex();
        auto type = index.data(Qt::UserRole).toString();
        if (type == QStringLiteral("separator")) {
            editWidget->setEnabled(false);
            folderEdit->setText(QString());
            folderEdit->setPlaceholderText(QString());
            iconPreview->setIcon(QIcon());
        } else {
            auto iconName = index.data(Qt::DecorationRole).value<QIcon>().name();
            editWidget->setEnabled(true);
            folderEdit->setText(index.data().toString());
            iconPreview->setIcon(QIcon::fromTheme(iconName));
        }
    });

    connect(iconPreview, &QPushButton::clicked, this, [=]() {
        auto item = m_foldersListView->selectedItems().first();
        auto oldIconName = item->data(Qt::DecorationRole).value<QIcon>().name();
        auto newIconName = iconDialog->getIcon();
        auto iconName = newIconName.isEmpty() ? oldIconName : newIconName;
        item->setIcon(QIcon::fromTheme(iconName));
        iconPreview->setIcon(QIcon::fromTheme(iconName));
    });

    connect(folderEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        auto item = m_foldersListView->selectedItems().first();
        auto type = item->data(Qt::UserRole).toString();
        if (type != QStringLiteral("separator")) {
            item->setText(text);
        }
    });

    connect(moveUpButton, &QPushButton::clicked, this, [=]() {
        auto index = selectedIndex();
        if (index.isValid() && index.row() > 0) {
            auto item = m_foldersListView->selectedItems().first();
            auto aboveItem = m_foldersListView->item(m_foldersListView->row(item) - 1);

            auto tmpFolder = aboveItem->text();
            auto tmpIcon = aboveItem->icon();

            aboveItem->setText(item->text());
            aboveItem->setIcon(item->icon());

            item->setText(tmpFolder);
            item->setIcon(tmpIcon);

            m_foldersListView->setCurrentItem(aboveItem);

            Q_EMIT changed();
        }
    });

    connect(moveDownButton, &QPushButton::clicked, this, [=]() {
        auto index = selectedIndex();
        if (index.isValid() && index.row() < m_foldersListView->count() - 1) {
            auto item = m_foldersListView->selectedItems().first();
            auto bellowItem = m_foldersListView->item(m_foldersListView->row(item) + 1);

            auto tmpFolder = bellowItem->text();
            auto tmpIcon = bellowItem->icon();

            bellowItem->setText(item->text());
            bellowItem->setIcon(item->icon());

            item->setText(tmpFolder);
            item->setIcon(tmpIcon);

            m_foldersListView->setCurrentItem(bellowItem);

            Q_EMIT changed();
        }
    });

    connect(addFolderButton, &QToolButton::clicked, this, [=]() {
        QString path = QFileDialog::getExistingDirectory(this, i18n("Select a folder"), QDir::homePath());
        if (!path.isEmpty()) {
            auto *item = new QListWidgetItem(QIcon::fromTheme(u"folder"_qs), path);
            item->setToolTip(i18n("Double click to set icon"));
            m_foldersListView->addItem(item);
            m_foldersListView->setCurrentItem(item);
            Q_EMIT changed();
        }
    });
    connect(deleteFolderButton, &QPushButton::clicked, this, [=]() {
        auto items = m_foldersListView->selectedItems();
        for(const auto& item : items) {
            delete m_foldersListView->takeItem(m_foldersListView->row(item));
        }

        Q_EMIT changed();
    });

    connect(m_foldersListView->model(), &QAbstractItemModel::rowsInserted, this, [=](const QModelIndex &, int, int last) {
        auto item = m_foldersListView->item(last);
        item->setSelected(true);
    });

    connect(m_foldersListView->model(), &QAbstractItemModel::rowsMoved, this, &FoldersSettingsPage::changed);
    connect(m_foldersListView->model(), &QAbstractItemModel::dataChanged, this, &FoldersSettingsPage::changed);
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
        QString type = modelIndex.data(Qt::UserRole).toString();

        auto group = m_config->group(QString(u"Folder_%1"_qs).arg(i));
        group.writeEntry(QStringLiteral("Path"), path);
        group.writeEntry(QStringLiteral("Icon"), iconName);
        group.writeEntry(QStringLiteral("Type"), type);
        m_config->sync();
    }
    auto group = m_config->group(u"General"_qs);
    group.writeEntry("FoldersCount", QString::number(foldersCount));
    m_config->sync();
}

void FoldersSettingsPage::deleteFolders()
{
    int foldersCount = m_config->group(u"General"_qs).readEntry("FoldersCount").toInt();
    // delete all folders
    for (int i = 0; i < foldersCount; ++i) {
        auto group = m_config->group(QString(u"Folder_%0"_qs).arg(i));
        m_config->deleteGroup(group.name());
    }
    m_config->deleteGroup(u"Folders"_qs);
    m_config->sync();
}
