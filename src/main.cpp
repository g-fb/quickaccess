#include "mainwindow.h"

#include <KLocalizedString>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationVersion("1.0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("QuickAccess");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption showTrayIconOption(
                QStringLiteral("tray-icon"),
                QCoreApplication::translate("main", "Set tray icon visibility.\nValues: show, hide. Default is show."
                                            "\n Example: quickaccess -tray-icon=hide"),
                QCoreApplication::translate("main", "visibility"), QStringLiteral("show"));
    parser.addOption(showTrayIconOption);

    parser.process(app);

    KLocalizedString::setApplicationDomain("quickaccess");

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                              "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    MainWindow *w = new MainWindow();

    QString showTrayIcon = parser.value(showTrayIconOption);
    if (showTrayIcon == "show") {
        w->createTrayIcon(true);
    } else if (showTrayIcon == "hide") {
        w->createTrayIcon(false);
    }

    return app.exec();
}

