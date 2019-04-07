#include "mainwindow.h"

#include <KAboutData>
#include <KLocalizedString>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("quickaccess");
    KAboutData aboutData(
        // The program name used internally. (componentName)
        QStringLiteral("quickaccess"),
        // A displayable program name string. (displayName)
        i18n("QuickAccess"),
        // The program version string. (version)
        QStringLiteral("1.0.0"),
        // Short description of what the app does. (shortDescription)
        i18n("Quick access to selected folders and their subfolders."),
        // The license this code is released under
        KAboutLicense::GPL,
        // Copyright Statement (copyrightStatement = QString())
        i18n("(c) 2019"),
        // Optional text shown in the About box.
        // Can contain any information desired. (otherText)
        i18n(""),
        // The program homepage string. (homePageAddress = QString())
        QStringLiteral("http://example.com"),
        // The bug report email address
        // (bugsEmailAddress = QLatin1String("submit@bugs.kde.org")
        QStringLiteral("bugs@example.com"));
    
    aboutData.addAuthor(
        i18n("George Florea Banus"),
        i18n("Developer"),
        QStringLiteral("name@example.org"),
        QStringLiteral("http://example.com")
    );
    KAboutData::setApplicationData(aboutData);
    
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                              "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);
    
    MainWindow *w = new MainWindow();
//     w->show();

    return app.exec();
}

