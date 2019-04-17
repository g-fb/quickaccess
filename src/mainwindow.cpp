#include "mainwindow.h"
#include "pathsmenu.h"
#include "quickaccessadaptor.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDesktopServices>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QStringListModel>
#include <QSystemTrayIcon>
#include <QVBoxLayout>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QDebug>

static QDBusConnection connection(QLatin1String(""));

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->hide();
    ui->menuBar->hide();
    ui->statusBar->hide();
    setWindowTitle("Manage Paths");
    
    m_config = KSharedConfig::openConfig("quickaccessrc");
    KConfigGroup pathsGroup = m_config->group("Paths");
    QStringList mPathsList = pathsGroup.readPathEntry("paths", QStringList());
    
    QWidget *mainWidget = new QWidget();
    setCentralWidget(mainWidget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    mainWidget->setLayout(vLayout);
    
    QLabel *label = new QLabel(i18n("Drag and drop items to reorder them."));
    QPushButton *addFolderBtn = new QPushButton(i18n("Add Folder"));
    connect(addFolderBtn, &QPushButton::clicked,
            this, &MainWindow::selectFolder);
    QPushButton *deleteFolderBtn = new QPushButton(i18n("Delete Folder"));
    deleteFolderBtn->setDisabled(true);
    connect(deleteFolderBtn, &QPushButton::clicked,
            this, &MainWindow::deleteFolder);
    
    QWidget *buttonsWidget = new QWidget();
    QHBoxLayout *buttonsHLayout = new QHBoxLayout();
    buttonsWidget->setLayout(buttonsHLayout);
    buttonsHLayout->addWidget(addFolderBtn);
    buttonsHLayout->addWidget(deleteFolderBtn);
    
    m_listWidget = new QListWidget();
    m_listWidget->setDragEnabled(true);
    m_listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    connect(m_listWidget->model(), &QAbstractItemModel::rowsMoved,
            this, &MainWindow::savePaths);
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, [=]() {
                deleteFolderBtn->setEnabled(true);
            });
    for (int i = 0; i < mPathsList.size(); i++) {
        m_listWidget->insertItem(i, mPathsList[i]);
    }
    
    vLayout->addWidget(label);
    vLayout->addWidget(m_listWidget);
    vLayout->addWidget(buttonsWidget);
    
    createTrayIcon();
    setupMenu();
    setupDBus();
}

MainWindow::~MainWindow() = default;

// Ads a menu or an action to menu
// if path is a folder with sub folders add a menu else add an action
void MainWindow::addMenuItem(QMenu *menu, QString path)
{
    QDirIterator it(path,
                    QDir::Dirs|QDir::NoDotAndDotDot,
                    QDirIterator::NoIteratorFlags);
    QStringList itFolders;
    while (it.hasNext()) {
        itFolders << it.next();
    }
    
    const QFileInfo file(path);
    if (file.isDir() && !itFolders.isEmpty()) {
        // folder has sub folders
        menu->addMenu(createMenu(path));
    } else {
        // folder has no sub folders
        QAction *action = new QAction();
        action->setText(path.split("/").takeLast());
        action->setIcon(QIcon::fromTheme("folder"));
        menu->addAction(action);
        connect(action, &QAction::triggered,
                this, [=]() {
                    actionClicked = true;
                    openFolder(path);
                });
    }
}

QMenu *MainWindow::createMenu(QString path)
{
    PathsMenu *menu = new PathsMenu();
    menu->setMinimumWidth(300);
    menu->setTitle(path.split("/").takeLast());
    menu->setIcon(QIcon::fromTheme("folder"));
    menu->setMainWindow(this);
        
    connect(menu, &PathsMenu::actionTriggered,
            this, [=]() {
                openFolder(path);
                mMenu->close();
            });
    connect(menu->menuAction(), &QAction::hovered,
            this, [=]() {
                onQMenuHover(menu, path);
            });
    return menu;
}

void MainWindow::createTrayIcon()
{
    AboutDialog *aboutDialog = new AboutDialog(nullptr);
    trayIconMenu = new QMenu(this);
    
    QAction *quitAction = new QAction(i18n("Quit"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(quitAction, &QAction::triggered,
            QCoreApplication::instance(), &QCoreApplication::quit);
    
    QAction *aboutAction = new QAction(i18n("About QuickAccess"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    connect(aboutAction, &QAction::triggered,
            aboutDialog, &AboutDialog::show);
    
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("QuickAccess");
    trayIcon->setIcon(QIcon::fromTheme("folder"));
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
}

void MainWindow::deleteFolder()
{
    if (m_listWidget->selectedItems().count() > 0) {
        int row = m_listWidget->row(m_listWidget->selectedItems().at(0));
        m_listWidget->model()->removeRow(row);
        savePaths();
    }
}

void MainWindow::onQMenuHover(QMenu *menu, QString path)
{
    if (menu->isEmpty()) {
        QDirIterator it(path,
                        QDir::Dirs|QDir::NoDotAndDotDot,
                        QDirIterator::NoIteratorFlags);
        QStringList m_allFiles;
        while (it.hasNext()) {
            m_allFiles << it.next();
        }
        QCollator collator;
        collator.setNumericMode(true);
        std::sort(m_allFiles.begin(), m_allFiles.end(), collator);
        for (int index = 0; index < m_allFiles.size(); index++) {
            if (index == 25) {
                menu->addSeparator();
                menu->addAction(new QAction(i18n("Folder has to many items.")));
                break;
            }
            addMenuItem(menu, m_allFiles[index]);
        }
    }
}

void MainWindow::openFolder(QString path)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::toNativeSeparators(path)));
}

QStringList MainWindow::paths()
{
    QStringList paths;
    for (int i = 0; i < m_listWidget->count(); i++) {
        paths << m_listWidget->item(i)->text();
    }
    return paths;
}

void MainWindow::savePaths()
{
    KConfigGroup pathsGroup = m_config->group("Paths");
    
    pathsGroup.writePathEntry("paths", paths());
    pathsGroup.config()->sync();
    setupMenu();
}

void MainWindow::selectFolder()
{
    QString path = QFileDialog::getExistingDirectory(
        this, i18n("Choose a directory"), QDir::homePath());
    if (path.isEmpty()) {
        return;
    }
    m_listWidget->insertItem(m_listWidget->count(), path);
    KConfigGroup pathsGroup = m_config->group("Paths");
    pathsGroup.writePathEntry("paths", paths());
    pathsGroup.config()->sync();
    setupMenu();
}

void MainWindow::setupDBus()
{
    new QuickAccessAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/QuickAccess", this);
    dbus.registerService("com.georgefb.QuickAccess");
}

void MainWindow::setupMenu()
{
    if (mMenu != nullptr) {
        mMenu->clear();
    }
    mMenu = new QMenu();
    mMenu->setObjectName("poopMenu");
    mMenu->setFixedWidth(300);

    for (int x = 0; x < paths().size(); x++) {
        addMenuItem(mMenu, paths().at(x));
    }
    // ----------------------------- //
    mMenu->addSeparator();
    
    QAction *action = new QAction();
    action->setObjectName("add_folder");
    action->setText(i18n("Add Folder"));
    connect(action, &QAction::triggered,
            this, [=]() {
                // see actionClicked in mainwindow.h
                actionClicked = true;
                selectFolder();
            });
    mMenu->addAction(action);
    
    action = new QAction();
    action->setText(i18n("Manage Paths"));
    connect(action, &QAction::triggered,
            this, [=]() {
                actionClicked = true;
                show();
            });
    mMenu->addAction(action);
    
    action = new QAction();
    action->setText(i18n("Close Menu"));
    connect(action, &QAction::triggered,
            this, [=]() {
                actionClicked = true;
                mMenu->close();
            });
    mMenu->addAction(action);
    
    action = new QAction();
    action->setText(i18n("Quit"));
    action->setIcon(QIcon::fromTheme("application-exit"));
    connect(action, &QAction::triggered, 
            this, [=]() {
                actionClicked = true;
                QCoreApplication::quit();
            });
    mMenu->addAction(action);
}

void MainWindow::showMenu()
{
    mMenu->exec(QCursor::pos());
}

// should be used when triggered by a double clicked
// without a delay the menu doesn't close when clicking outside it
void MainWindow::showDelayedMenu(int delay)
{
    QThread::msleep(delay);
    mMenu->exec(QCursor::pos());
}
