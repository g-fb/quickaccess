#include "mainwindow.h"

#include <KLocalizedString>
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationVersion("3.0.0");
    KLocalizedString::setApplicationDomain("quickaccess");
    QApplication::setQuitOnLastWindowClosed(false);
    MainWindow w;

    return app.exec();
}
