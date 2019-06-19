#include "mainwindow.h"

#include <KLocalizedString>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("quickaccess");

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                              "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);
    
    MainWindow *w = new MainWindow();

    return app.exec();
}

