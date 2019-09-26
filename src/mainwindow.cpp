#include "mainwindow.h"
#include "pathsmenu.h"
#include "quickaccessadaptor.h"
#include "settings.h"

#include <QCommandLineParser>
#include <QFileDialog>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDesktopServices>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QStringListModel>
#include <QSystemTrayIcon>
#include <QVBoxLayout>

#include <KConfig>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KLocalizedString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings(new Settings(nullptr))
    , m_config(KSharedConfig::openConfig("quickaccessrc"))
    , mMenu(new QMenu())
{
    QCommandLineParser parser;
    parser.setApplicationDescription("QuickAccess");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption showTrayIconOption(
                QStringLiteral("tray-icon"),
                QCoreApplication::translate("main", "Set tray icon visibility.\nValues: show, hide. Default is show."
                                            "\n Example: quickaccess --tray-icon=hide"),
                QCoreApplication::translate("main", "visibility"), QStringLiteral("show"));
    parser.addOption(showTrayIconOption);
    parser.process(QCoreApplication::instance()->arguments());

    QString showTrayIcon = parser.value(showTrayIconOption);
    if (showTrayIcon == "show") {
        createTrayIcon(true);
    } else if (showTrayIcon == "hide") {
        createTrayIcon(false);
    }

    setupMenu();
    setupDBus();
}

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
    connect(menu, &QMenu::aboutToShow,
            this, [=]() {
                onQMenuHover(menu, path);
            });
    return menu;
}

void MainWindow::createTrayIcon(bool show)
{
    if (!show) {
        return;
    }
    AboutDialog *aboutDialog = new AboutDialog(nullptr);
    aboutDialog->setWindowIcon(QIcon::fromTheme("quickaccess", QIcon::fromTheme("folder")));
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
    trayIcon->setIcon(QIcon::fromTheme("quickaccess", QIcon::fromTheme("folder")));
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
}

void MainWindow::onQMenuHover(QMenu *menu, QString path)
{
    menu->clear();
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

void MainWindow::openFolder(QString path)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::toNativeSeparators(path)));
}

void MainWindow::openSettings()
{
    if (KConfigDialog::showDialog("settings")) {
        return;
    }
    auto dialog = new KConfigDialog(
             this, "settings", QuickAccessSettings::self());
    dialog->setMinimumSize(700, 600);
    dialog->setFaceType(KPageDialog::Plain);
    dialog->addPage(m_settings, i18n("Settings"));
    connect(dialog, &KConfigDialog::settingsChanged, this, &MainWindow::setupMenu);
    dialog->show();

    // add button to open file dialog to select a folder
    // and add it to the folders list
    auto addFolderButton = new QPushButton(i18n("Select and Add Folder"));
    addFolderButton->setIcon(QIcon::fromTheme("folder-add"));
    connect(addFolderButton, &QPushButton::clicked, this, [=]() {
        selectFolder();
        emit m_settings->kcfg_paths->changed();
    });
    auto widget = new QWidget();
    auto hLayout = new QHBoxLayout(widget);
    hLayout->setMargin(0);
    hLayout->addWidget(addFolderButton);
    hLayout->addStretch(1);
    // add widget to the keditlistwidget's layout
    m_settings->kcfg_paths->layout()->addWidget(widget);

}

void MainWindow::selectFolder()
{
    QString path = QFileDialog::getExistingDirectory(
        this, i18n("Choose a directory"), QDir::homePath());
    if (path.isEmpty()) {
        return;
    }
    m_settings->kcfg_paths->insertItem(path, m_settings->kcfg_paths->count());
    setupMenu();
}

void MainWindow::setupDBus()
{
    new QuickAccessAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/QuickAccess", this);
    dbus.registerService("com.georgefb.quickaccess");
}

void MainWindow::setupMenu()
{
    mMenu->clear();
    mMenu->setObjectName("mainMenu");
    mMenu->setFixedWidth(300);

    auto paths = m_config->group("Paths").readPathEntry("paths", QStringList());
    for (auto path : paths) {
        addMenuItem(mMenu, path);
    }
    // ----------------------------- //
    mMenu->addSeparator();

    auto *action = new QAction();
    action->setObjectName("add_folder");
    action->setText(i18n("Add Folder"));
    connect(action, &QAction::triggered, this, [=]() {
        // see actionClicked in mainwindow.h
        actionClicked = true;
        selectFolder();
    });
    mMenu->addAction(action);

    action = new QAction();
    action->setText(i18n("Manage Paths"));
    action->setIcon(QIcon::fromTheme("configure"));
    connect(action, &QAction::triggered, this, [=]() {
        actionClicked = true;
        openSettings();
    });
    mMenu->addAction(action);

    action = new QAction();
    action->setText(i18n("Close Menu"));
    connect(action, &QAction::triggered, this, [=]() {
        actionClicked = true;
        mMenu->close();
    });
    mMenu->addAction(action);

    action = new QAction();
    action->setText(i18n("Quit"));
    action->setIcon(QIcon::fromTheme("application-exit"));
    connect(action, &QAction::triggered, this, [=]() {
        actionClicked = true;
        QCoreApplication::quit(), Qt::QueuedConnection;
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
