#include "mainwindow.h"
#include "pathsmenu.h"
#include "quickaccessadaptor.h"
#include "settings.h"
#include "settingsdialog.h"

#include <QClipboard>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDesktopServices>
#include <QFileDialog>
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
#include <KShell>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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
    openSettings();
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
    if (file.isDir() && !itFolders.isEmpty() && QuickAccessSettings::submenuEntriesCount() != 0) {
        // folder has sub folders, create menu
        auto *submenu = new PathsMenu();
        submenu->setMinimumWidth(200);
        submenu->setMaximumWidth(600);
        QFontMetrics metrix(submenu->font());
        QString elidedTitle = metrix.elidedText(path.split("/").takeLast(), Qt::ElideRight, 500);
        submenu->setTitle(elidedTitle);
        submenu->setIcon(QIcon::fromTheme("folder"));
        submenu->setMainWindow(this);

        connect(submenu, &PathsMenu::actionTriggered, this, [=]() {
            openFolder(path);
            mMenu->close();
        });
        connect(submenu, &QMenu::aboutToShow, this, [=]() {
            onMenuHover(submenu, path);
        });
        menu->addMenu(submenu);
    } else {
        // folder has no sub folders, create action
        auto *action = new QAction();
        action->setText(path.split("/").takeLast());
        action->setIcon(QIcon::fromTheme("folder"));
        menu->addAction(action);
        connect(action, &QAction::triggered, this, [=]() {
            actionClicked = true;
            openFolder(path);
        });
    }
}

void MainWindow::createTrayIcon(bool show)
{
    if (!show) {
        return;
    }
    auto *aboutDialog = new AboutDialog(nullptr);
    aboutDialog->setWindowIcon(QIcon::fromTheme("quickaccess", QIcon::fromTheme("folder")));
    trayIconMenu = new QMenu(this);

    auto *quitAction = new QAction(i18n("Quit"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(quitAction, &QAction::triggered,
            QCoreApplication::instance(), &QCoreApplication::quit);

    auto *aboutAction = new QAction(i18n("About QuickAccess"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    connect(aboutAction, &QAction::triggered,
            aboutDialog, &AboutDialog::show);

    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    auto *trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("QuickAccess");
    trayIcon->setIcon(QIcon::fromTheme("quickaccess", QIcon::fromTheme("folder")));
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
}

void MainWindow::onMenuHover(QMenu *menu, QString path)
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
        if (index == QuickAccessSettings::submenuEntriesCount()) {
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
    m_settingsDialog = new SettingsDialog(nullptr, "settings", QuickAccessSettings::self());
    m_settingsDialog->setMinimumSize(700, 750);
    m_settingsDialog->setFaceType(KPageDialog::Plain);
    connect(m_settingsDialog, &SettingsDialog::settingsChanged, this, &MainWindow::setupMenu);
    m_settingsDialog->show();
}

void MainWindow::setupDBus()
{
    new QuickAccessAdaptor(this);
    auto dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/QuickAccess", this);
    dbus.registerService("com.georgefb.quickaccess");
}

void MainWindow::setupMenu()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    mMenu->clear();
    mMenu->setObjectName("mainMenu");
    mMenu->setMinimumWidth(200);
    mMenu->setMaximumWidth(350);

    auto paths = m_config->group("Paths").readPathEntry("paths", QStringList());
    for (auto path : paths) {
        addMenuItem(mMenu, path);
    }
    mMenu->addSeparator();
    // ----------------------------- //

    int commandsCount = m_config->group("Commands").readEntry("Count").toInt();
    for (int i = 0; i < commandsCount; i++) {
        auto group = m_config->group(QString("Command_%1").arg(i));
        if (group.readEntry("Type") == "menu") {
            int menuCount = group.readEntry("Count").toInt();
            if (menuCount < 1) {
                continue;
            }
            auto menu = new QMenu();
            menu->setTitle(group.readEntry("Name"));
            menu->setMinimumWidth(200);
            menu->setIcon(QIcon::fromTheme(group.readEntry("Icon")));
            for (int j = 0; j < menuCount; ++j) {
                auto group = m_config->group(QString("Command_%1__Action_%2").arg(i).arg(j));
                auto action = new QAction();
                action->setText(group.readEntry("Name"));
                action->setIcon(QIcon::fromTheme(group.readEntry("Icon")));
                connect(action, &QAction::triggered, [=]() {
                    auto args = group.readEntry("Args");
                    args.replace("{clipboard}", clipboard->text());
                    auto *process = new QProcess();
                    process->setProgram(group.readEntry("Process"));
                    process->setArguments(KShell::splitArgs(args));
                    process->start();
                });
                menu->addAction(action);
            }
            mMenu->addMenu(menu);
        } else {
            auto action = new QAction();
            action->setIcon(QIcon::fromTheme(group.readEntry("Icon")));
            action->setText(group.readEntry("Name"));
            connect(action, &QAction::triggered, [=]() {
                auto args = group.readEntry("Args");
                args.replace("{clipboard}", clipboard->text());
                auto *process = new QProcess();
                process->setProgram(group.readEntry("Process"));
                process->setArguments(KShell::splitArgs(args));
                process->start();
            });
            mMenu->addAction(action);
        }
    }
    mMenu->addSeparator();

    auto action = new QAction();
    action->setText(i18n("Settings"));
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

    mMenu->addSeparator();
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
