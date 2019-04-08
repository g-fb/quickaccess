#ifndef MAINWINDOW
#define MAINWINDOW

#include <QMainWindow>
#include <KSharedConfig>
#include <QSystemTrayIcon>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.georgefb.QuickAccess")

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    Q_SCRIPTABLE void showMenu();
    Q_SCRIPTABLE void showDelayedMenu(int delay = 150);
    
private:
    Ui::MainWindow *ui;
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
