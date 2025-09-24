#include "mainwindow.h"

#include <QApplication>

#include <KLocalizedString>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon::fromTheme(u"quickaccess"_s, QIcon(u":/icons/sc-apps-quickaccess"_s)));
    QCoreApplication::setApplicationVersion(u"3.0.0"_s);
    KLocalizedString::setApplicationDomain("quickaccess");
    QApplication::setQuitOnLastWindowClosed(false);
    MainWindow w;

    return app.exec();
}
