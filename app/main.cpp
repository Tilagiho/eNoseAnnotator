#include "lib/QCrashHandler/src/qcrashhandler.h"
#include <QApplication>
#include "classes/controler.h"

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


    // init Controler
    Controler c;

    // timer to check arguments
    QTimer::singleShot(0, &c, &Controler::initialize);

    // start applivation
    c.getWindow()->show();
    return a.exec();
}
