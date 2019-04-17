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
class QListView;
class QMenu;
class QDBusConnectionInterface;
class QStringListModel;
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
    Q_CLASSINFO("D-Bus Interface", "com.georgefb.QuickAccess")

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    Q_SCRIPTABLE void showMenu();
    Q_SCRIPTABLE void showDelayedMenu(int delay = 150);
    
    // actionClicked - prevent triggering of wrong action
    // clicking a normal QAction can trigger the action of PathsMenu
    // when the interval of moving the mouse from PathsMenu to a normal QAction is very small
    // this is a workaround
    bool actionClicked = false;
    
private:
    Ui::MainWindow *ui;
    void addMenuItem(QMenu *menu, QString path);
    void selectFolder();
    void deleteFolder();
    void setDolphinDBusService(QDBusConnectionInterface* bus);
    void onQMenuHover(QMenu *menu, QString path);
    void showMainWindow();
    void savePaths();
    void openFolder(QString path);
    void setupMenu();
    void setupDBus();
    void createTrayIcon();
    QStringList paths();
    QMenu *createMenu(QString path);
    QMenu *mMenu = nullptr;
    QListWidget *m_listWidget;
    QDBusConnectionInterface *bus;
    QMenu *trayIconMenu;
    KSharedConfig::Ptr m_config;
};

#endif // MAINWINDOW
