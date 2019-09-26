#ifndef MAINWINDOW
#define MAINWINDOW

#include <QMainWindow>
#include <KSharedConfig>
#include <QSystemTrayIcon>

#include "ui_AboutDialog.h"
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
    Q_SCRIPTABLE void showMenu();
    Q_SCRIPTABLE void showDelayedMenu(int delay = 150);

    // actionClicked - prevent triggering of wrong action
    // clicking a normal QAction can trigger the action of PathsMenu
    // when the interval of moving the mouse from PathsMenu to a normal QAction is very small
    // this is a workaround
    bool actionClicked = false;
    void createTrayIcon(bool show);

private:
    Ui::MainWindow *ui;
    void addMenuItem(QMenu *menu, QString path);
    void selectFolder();
    void onMenuHover(QMenu *menu, QString path);
    void openFolder(QString path);
    void openSettings();
    void setupMenu();
    void setupDBus();
    QMenu *mMenu = nullptr;
    QDBusConnectionInterface *bus;
    QMenu *trayIconMenu;
    Settings *m_settings;
    KSharedConfig::Ptr m_config;
};

#endif // MAINWINDOW
