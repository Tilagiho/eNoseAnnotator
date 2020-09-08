#include "widgets/mainwindow.h"

#include "lib/QCrashHandler/src/qcrashhandler.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("eNoseAnnotator");

    // init application settings
    QCoreApplication::setOrganizationName("smart nanotubes GmbH");
//    QCoreApplication::setOrganizationDomain("mysoft.com");
    QCoreApplication::setApplicationName("eNoseAnnotator");

    // setup breakpad crash handler:
    // save crash minidumps in reports
    QString gitCommit(GIT_VERSION);
    QString reportPath = QDir::tempPath() + "/" + QCoreApplication::applicationName() + "/crash_reports/" + gitCommit;
    QDir reportDir(reportPath);
    if (!reportDir.exists()) // create reportPath if necessary
        QDir().mkpath(reportDir.absolutePath());

    Breakpad::CrashHandler::instance()->Init(reportDir.absolutePath());

    // start application
    MainWindow w;

    // timer to check aarguments
    QTimer::singleShot(0, &w, SLOT(initialize()));

    w.show();
    return a.exec();
}
