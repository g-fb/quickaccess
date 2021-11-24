#ifndef MAINWINDOW
#define MAINWINDOW

#include <QMainWindow>
#include <KSharedConfig>
#include <QSystemTrayIcon>

#include "ui_aboutdialog.h"
#include "ui_startupdialog.h"


class SettingsWindow;

class QVBoxLayout;
class QPushButton;
class QMenu;
class QDBusConnectionInterface;
class QListWidget;
class KConfigDialog;
class KConfigGroup;

class AboutDialog: public QWidget, public Ui::AboutDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent)
    : QWidget(parent)
    {
        setupUi(this);
    }
};


class StartUpDialog: public QDialog, public Ui::StartUpDialog
{
    Q_OBJECT
public:
    explicit StartUpDialog(QWidget *parent) : QDialog(parent) {
        setupUi(this);
    }
};


class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.georgefb.quickaccess")

public:
    explicit MainWindow(QWidget *parent = nullptr);
    Q_SCRIPTABLE void showMenu(int pos = 0);
    Q_SCRIPTABLE void showDelayedMenu(unsigned long delay = 150, int pos = 0);

    // actionClicked - prevent triggering of wrong action
    // clicking a normal QAction can trigger the action of PathsMenu
    // when the interval of moving the mouse from PathsMenu to a normal QAction is very small
    // this is a workaround
    bool actionClicked = false;
    void createTrayIcon(bool show);

signals:
    void addFolder(QString path);

private:
    void addMenuItem(QMenu *menu, QString path, QString iconName = "folder");
    void onMenuHover(QMenu *menu, QString path);
    void openFolder(QString path);
    void setupMenu();
    void setupDBus();
    bool isRunningSandbox();

    QIcon m_appIcon;
    QMenu *m_menu = nullptr;
    QDBusConnectionInterface *bus;
    QSystemTrayIcon *m_trayIcon;
    KSharedConfig::Ptr m_config;

    QAction *createCustomCommand(KConfigGroup group);
    QClipboard *m_clipboard;
};

#endif // MAINWINDOW
