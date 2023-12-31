#include "mainwindow.h"
#include "pathsmenu.h"
#include "quickaccessadaptor.h"
#include "migrations.h"
#include "settings.h"
#include "settings/settingswindow.h"

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
#include <QScreen>
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
    , m_menu(new QMenu())
    , m_config(KSharedConfig::openConfig("quickaccessrc"))
{
    // call adjustSize so the settings window opens in the center of the screen
    adjustSize();

    Migrations migrations;
    migrations.migrate();

    m_clipboard = QGuiApplication::clipboard();

    m_startUpDialog = new StartUpDialog(this);
    m_startUpDialog->setWindowTitle(QStringLiteral("QuickAccess"));
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - m_startUpDialog->width()) / 2;
    int y = (screenGeometry.height() - m_startUpDialog->height()) / 2;
    m_startUpDialog->move(screenGeometry.x() + x, screenGeometry.y() + y);
    if (m_config->group("General").readEntry("ShowStartUpDialog", true)) {
        m_startUpDialog->kcfg_ShowOnStartUp->setChecked(m_config->group("General").readEntry("ShowStartUpDialog", true));
        m_startUpDialog->show();
    }
    connect(m_startUpDialog->openMenuButton, &QPushButton::clicked,
            this, &MainWindow::showMenu);
    connect(m_startUpDialog->openSettingsButton, &QPushButton::clicked, this, [=]() {
        openSettings();
    });
    connect(m_startUpDialog->copyCommandButton, &QPushButton::clicked, this, [=]() {
        m_clipboard->setText(QStringLiteral("dbus-send --type=method_call --dest=com.georgefb.quickaccess /QuickAccess com.georgefb.QuickAccess.showMenu"));
    });
    connect(m_startUpDialog->kcfg_ShowOnStartUp, &QCheckBox::stateChanged, this, [=]() {
        m_config->group("General").writeEntry("ShowStartUpDialog", m_startUpDialog->kcfg_ShowOnStartUp->isChecked());
        m_config->sync();
    });

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

    if (isRunningSandbox()) {
        m_appIcon = QIcon::fromTheme("com.georgefb.quickaccess", QIcon(":/icons/quickaccess"));
    } else {
        m_appIcon = QIcon::fromTheme("quickaccess", QIcon(":/icons/quickaccess"));
    }

    QString showTrayIcon = parser.value(showTrayIconOption);
    if (showTrayIcon == "show" && QuickAccessSettings::showInTray()) {
        createTrayIcon(true);
    } else if (showTrayIcon == "hide" || !QuickAccessSettings::showInTray()) {
        createTrayIcon(false);
    }

    setupMenu();
    setupDBus();
}

// Ads a menu or an action to menu
// if path is a folder with sub folders add a menu else add an action
void MainWindow::addMenuItem(QMenu *menu, QString path, QString iconName)
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
        QStringList pathFolders = path.split("/");
        pathFolders.removeAll(QString(""));
        QString elidedTitle = metrix.elidedText(pathFolders.takeLast(), Qt::ElideRight, 500);
        submenu->setTitle(elidedTitle);
        submenu->setIcon(QIcon::fromTheme(iconName));
        submenu->setMainWindow(this);

        connect(submenu, &PathsMenu::actionTriggered, this, [=]() {
            openFolder(path);
            m_menu->close();
        });
        connect(submenu, &QMenu::aboutToShow, this, [=]() {
            onMenuHover(submenu, path);
        });
        menu->addMenu(submenu);
    } else {
        // folder has no sub folders, create action
        auto *action = new QAction(nullptr);
        QFontMetrics metrix(action->font());
        QStringList pathFolders = path.split("/");
        pathFolders.removeAll(QString(""));
        QString elidedTitle = metrix.elidedText(pathFolders.takeLast(), Qt::ElideRight, 500);
        action->setText(elidedTitle);
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
    auto *aboutDialog = new AboutDialog(nullptr);
    aboutDialog->setWindowIcon(m_appIcon);
    auto trayIconMenu = new QMenu(this);

    auto *quitAction = new QAction(i18n("Quit"), nullptr);
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(quitAction, &QAction::triggered,
            QCoreApplication::instance(), &QCoreApplication::quit);

    auto *aboutAction = new QAction(i18n("About QuickAccess"), nullptr);
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    connect(aboutAction, &QAction::triggered,
            aboutDialog, &AboutDialog::show);

    auto *settingsAction = new QAction(i18n("Settings"), nullptr);
    settingsAction->setIcon(QIcon::fromTheme("configure"));
    connect(settingsAction, &QAction::triggered, this, [=]() {
        openSettings();
    });

    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("QuickAccess");
    m_trayIcon->setIcon(m_appIcon);
    m_trayIcon->setContextMenu(trayIconMenu);
    m_trayIcon->setVisible(show);
}

void MainWindow::openSettings()
{
    if (SettingsWindow::showDialog(QStringLiteral("settings"))) {
        return;
    }
    auto settingsWindow = new SettingsWindow(this, QuickAccessSettings::self());
    settingsWindow->show();
    connect(settingsWindow, &SettingsWindow::openStartUpDialog, this, [=]() {
        m_startUpDialog->kcfg_ShowOnStartUp->setChecked(m_config->group("General").readEntry("ShowStartUpDialog", true));
        m_startUpDialog->show();
    });
    connect(settingsWindow, &SettingsWindow::settingsChanged, this, &MainWindow::setupMenu);
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
        int maxSubItems = QuickAccessSettings::submenuEntriesCount();
        if (index == maxSubItems) {
            menu->addSeparator();
            auto action = new QAction(nullptr);
            action->setText(i18n("There are more folders than configured to show (%1).", maxSubItems));
            connect(action, &QAction::triggered, this, [=]() {
                actionClicked = true;
                KConfigDialog::showDialog("settings");
            });
            menu->addAction(action);
            break;
        }
        addMenuItem(menu, m_allFiles[index]);
    }
}

void MainWindow::openFolder(QString path)
{
    if (isRunningSandbox()) {
        auto args = QStringList() << QStringLiteral("--host") << QStringLiteral("xdg-open") << path;
        auto *process = new QProcess();
        process->setProgram(QStringLiteral("flatpak-spawn"));
        process->setArguments(args);
        process->start();
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::toNativeSeparators(path)));
    }
}

void MainWindow::setupDBus()
{
    new QuickAccessAdaptor(this);
    auto dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/QuickAccess", this);
    dbus.registerService("com.georgefb.quickaccess");
}

QAction *MainWindow::createCustomCommand(KConfigGroup group)
{
    auto action = new QAction(nullptr);
    action->setText(group.readEntry("Name"));
    action->setIcon(QIcon::fromTheme(group.readEntry("Icon")));
    connect(action, &QAction::triggered, this, [=]() {
        QString processName = group.readEntry("Process");
        QString argsString = group.readEntry("Args");
        argsString.replace("{clipboard}", KShell::quoteArg(m_clipboard->text()));
        QStringList args = KShell::splitArgs(argsString);
        if (isRunningSandbox()) {
            processName = QStringLiteral("flatpak-spawn");
            args = QStringList() << QStringLiteral("--host") << group.readEntry("Process") << args;
        }
        auto *process = new QProcess();
        process->setProgram(processName);
        process->setArguments(args);
        process->start();
    });

    return action;
}

void MainWindow::setupMenu()
{
    m_menu->clear();
    m_menu->setObjectName("mainMenu");
    m_menu->setMinimumWidth(200);
    m_menu->setMaximumWidth(350);

    if (QuickAccessSettings::useSections()) {
        m_menu->addSection(i18n("Folders"));
    }

    auto generalGroup = m_config->group(QStringLiteral("General"));
    int foldersCount = generalGroup.readEntry("FoldersCount").toInt();
    for (int i = 0; i < foldersCount; ++i) {
        auto group = m_config->group(QStringLiteral("Folder_%1").arg(i));
        auto path = group.readEntry(QStringLiteral("Path"));
        auto iconName = group.readEntry(QStringLiteral("Icon"));
        auto type = group.readEntry(QStringLiteral("Type"));

        if (type == QStringLiteral("separator")) {
            m_menu->addSeparator();
        } else {
            addMenuItem(m_menu, path, iconName);
        }
    }
    if (QuickAccessSettings::useSections()) {
        m_menu->addSection(i18n("Commands"));
    } else {
        m_menu->addSeparator();
    }
    // ----------------------------- //

    int commandsCount = generalGroup.readEntry("CommandsCount").toInt();
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
                auto group = m_config->group(QString("Command_%1__Subcommand_%2").arg(i).arg(j));
                auto action = createCustomCommand(group);
                menu->addAction(action);
            }
            m_menu->addMenu(menu);
        } else {
            auto action = createCustomCommand(group);
            m_menu->addAction(action);
        }
    }
    m_menu->addSeparator();

    auto action = new QAction(nullptr);
    action->setText(i18n("Settings"));
    action->setIcon(QIcon::fromTheme("configure"));
    connect(action, &QAction::triggered, this, [=]() {
        actionClicked = true;
        openSettings();
    });
    m_menu->addAction(action);

    action = new QAction(nullptr);
    action->setText(i18n("Close Menu"));
    connect(action, &QAction::triggered, this, [=]() {
        actionClicked = true;
        m_menu->close();
    });
    m_menu->addAction(action);

    m_menu->addSeparator();
    action = new QAction(nullptr);
    action->setText(i18n("Quit"));
    action->setIcon(QIcon::fromTheme("application-exit"));
    connect(action, &QAction::triggered, this, [=]() {
        actionClicked = true;
        QCoreApplication::quit();
    }, Qt::QueuedConnection);
    m_menu->addAction(action);
}

void MainWindow::showMenu(int pos)
{
    m_menu->adjustSize();
    auto screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    int rightEdge = screenGeometry.width() - m_menu->rect().width() - 10;
    int bottomEdge = screenGeometry.height() - m_menu->rect().height() - 10;
    int xCenter = screenGeometry.center().x() - m_menu->rect().center().x();
    int yCenter = screenGeometry.center().y() - m_menu->rect().center().y();

    switch (pos) {
    case 0:
        m_menu->exec(QCursor::pos());
        break;
    case 1:
        m_menu->exec(QPoint(10, 10));
        break;
    case 2:
        m_menu->exec(QPoint(xCenter, 10));
        break;
    case 3:
        m_menu->exec(QPoint(rightEdge, 10));
        break;
    case 4:
        m_menu->exec(QPoint(10, yCenter));
        break;
    case 5:
        m_menu->exec(QPoint(xCenter, yCenter));
        break;
    case 6:
        m_menu->exec(QPoint(rightEdge, yCenter));
        break;
    case 7:
        m_menu->exec(QPoint(10, bottomEdge));
        break;
    case 8:
        m_menu->exec(QPoint(xCenter, bottomEdge));
        break;
    case 9:
        m_menu->exec(QPoint(rightEdge, bottomEdge));
        break;
    default:
        break;
    }
}

// should be used when triggered by a double clicked
// without a delay the menu doesn't close when clicking outside it
void MainWindow::showDelayedMenu(unsigned long delay, int pos)
{
    QThread::msleep(delay);
    showMenu(pos);
}

bool MainWindow::isRunningSandbox()
{
    QString runtimeDir = qgetenv("XDG_RUNTIME_DIR");

    if (runtimeDir.isEmpty()) {
        return false;
    }

    QFile file(runtimeDir + QLatin1String("/flatpak-info"));

    return file.exists();
}
