#ifndef MAINWINDOW
#define MAINWINDOW

#include <QMainWindow>
#include <KSharedConfig>
#include <QSystemTrayIcon>

#include "ui_aboutdialog.h"
#include "ui_settings.h"

namespace Ui {
    class MainWindow;
}

class QVBoxLayout;
class QPushButton;
class QMenu;
class QDBusConnectionInterface;
class QListWidget;
class KConfigDialog;
class KConfigGroup;

class Settings: public QWidget, public Ui::Settings
{
    Q_OBJECT
public:
    explicit Settings(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

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
    Ui::MainWindow *ui;
    void addMenuItem(QMenu *menu, QString path);
    void onMenuHover(QMenu *menu, QString path);
    void openFolder(QString path);
    void setupMenu();
    void setupDBus();
    bool isRunningSandbox();
    QIcon m_appIcon;
    QMenu *m_menu = nullptr;
    QDBusConnectionInterface *bus;
    QMenu *trayIconMenu;
    KSharedConfig::Ptr m_config;
    KConfigDialog *m_settingsDialog;
    Settings *m_settings;
    QAction *createCustomCommand(KConfigGroup group);
    QClipboard *m_clipboard;
};

#endif // MAINWINDOW
