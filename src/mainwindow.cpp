#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pathsmenu.h"

#include <QFileDialog>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDesktopServices>
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
#include <quickaccessadaptor.h>
#include <QLabel>

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

QMenu *MainWindow::createMenu(QString path)
{
    PathsMenu *menu = new PathsMenu();
    menu->setMinimumWidth(300);
    menu->setTitle(path.split("/").takeLast());
    menu->setIcon(QIcon::fromTheme("folder"));
    
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
    trayIconMenu = new QMenu(this);
    QAction *quitAction = new QAction(i18n("Quit"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    trayIconMenu->addAction(quitAction);
    trayIconMenu->addSeparator();
    
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("QuickAccess");
    trayIcon->setIcon(QIcon::fromTheme("folder"));
    trayIcon->show();
    trayIcon->setContextMenu(trayIconMenu);
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
            const QFileInfo file(m_allFiles[index]);
            
            QDirIterator it(m_allFiles[index],
                            QDir::Dirs|QDir::NoDotAndDotDot,
                            QDirIterator::NoIteratorFlags);
            QStringList itFiles;
            while (it.hasNext()) {
                itFiles << it.next();
            }
            if (file.isDir() && !itFiles.isEmpty()) {
                menu->addMenu(createMenu(m_allFiles[index]));
            } else {
                QAction *action = new QAction();
                action->setText(m_allFiles[index].split("/").takeLast());
                action->setIcon(QIcon::fromTheme("folder"));
                menu->addAction(action);
                connect(action, &QAction::triggered,
                        this, [=]() {
                            openFolder(m_allFiles[index]);
                            mMenu->close();
                        });
            }
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

void MainWindow::setDolphinDBusService(QDBusConnectionInterface *bus)
{
    const QStringList services = bus->registeredServiceNames();
    QMap<QString, QStringList> servicesWithAliases;
    
    int index = services.indexOf(QRegExp("^org.kde.dolphin.+"));
    if (index != -1) {
        m_dolphinServiceName = services[index];
        m_serviceWatcher = new QDBusServiceWatcher(services[index],
                                                   bus->connection(),
                                                   QDBusServiceWatcher::WatchForOwnerChange);
        connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged,
                this, [=]() {
                    m_dolphinServiceName = "";
                });
    }
}

void MainWindow::setupDBus()
{
    new QuickAccessAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/QuickAccess", this);
    dbus.registerService("com.georgefb.QuickAccess");
    
    connection = QDBusConnection::sessionBus();
    bus = connection.interface();
    setDolphinDBusService(bus);
}

void MainWindow::setupMenu()
{
    if (mMenu) {
        delete mMenu;
    }
    mMenu = new QMenu();
    mMenu->setFixedWidth(300);
    for (int x = 0; x < paths().size(); x++) {
        mMenu->addMenu(createMenu(paths()[x]));
    }
    mMenu->addSeparator();
    QAction *action = new QAction(QStringLiteral("add_folder"));
    action->setText(i18n("Add Folder"));
    connect(action, &QAction::triggered,
            this, &MainWindow::selectFolder);
    mMenu->addAction(action);
    
    action = new QAction(QStringLiteral("manage_paths"));
    action->setText(i18n("Manage Paths"));
    connect(action, &QAction::triggered,
            this, &MainWindow::show);
    mMenu->addAction(action);
    
    QAction *quitAction = new QAction(i18n("Quit"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    mMenu->addAction(quitAction);    
}

void MainWindow::showMenu()
{
    QThread::msleep(150);
    mMenu->exec(QCursor::pos());
}
