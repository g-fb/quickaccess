#include "mainwindow.h"

#include <QApplication>

#include <KLocalizedString>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(u"3.0.0"_s);
    KLocalizedString::setApplicationDomain("quickaccess");
    QApplication::setQuitOnLastWindowClosed(false);
    MainWindow w;

    return app.exec();
}
