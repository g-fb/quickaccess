#ifndef MAINWINDOW
#define MAINWINDOW

#include <QMainWindow>
#include <KSharedConfig>
#include <QSystemTrayIcon>

#include "ui_AboutDialog.h"

namespace Ui {
    class MainWindow;
}

class QVBoxLayout;
class QPushButton;
class QMenu;
class QDBusConnectionInterface;
class QListWidget;

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
    void deleteFolder();
    void onQMenuHover(QMenu *menu, QString path);
    void savePaths();
    void openFolder(QString path);
    void setupMenu();
    void setupDBus();
    QStringList paths();
    QMenu *createMenu(QString path);
    QMenu *mMenu = nullptr;
    QListWidget *m_listWidget;
    QDBusConnectionInterface *bus;
    QMenu *trayIconMenu;
    KSharedConfig::Ptr m_config;
};

#endif // MAINWINDOW
