#ifndef MAINWINDOW
#define MAINWINDOW

#include <QMainWindow>
#include <KSharedConfig>

namespace Ui {
    class MainWindow;
}

class QVBoxLayout;
class QPushButton;
class QListView;
class QMenu;
class QDBusConnectionInterface;
class QStringListModel;
class QDBusServiceWatcher;
class QListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "quickaccess.QuickAccess")

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    Q_SCRIPTABLE void showMenu();
    
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
    QMenu *createMenu(QString path);
    QStringList paths();
    QListWidget *m_listWidget;
    QString m_dolphinServiceName;
    QDBusServiceWatcher *m_serviceWatcher;
    QDBusConnectionInterface *bus;
    QMenu *trayIconMenu;
    KSharedConfig::Ptr m_config;
    QMenu       *mMenu = nullptr;
};

#endif // MAINWINDOW
