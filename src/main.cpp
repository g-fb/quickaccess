#include "mainwindow.h"

#include <KLocalizedString>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationVersion("2.0.9");
    KLocalizedString::setApplicationDomain("quickaccess");
    QApplication::setQuitOnLastWindowClosed(false);
    MainWindow w;

    return app.exec();
}
