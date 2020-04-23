#include "widgets/mainwindow.h"

#include "lib/QCrashHandler/src/qcrashhandler.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("eNoseAnnotator");

    // setup breakpad crash handler:
    // save crash minidumps in reports
    QDir reportDir ("crash reports/");
    if (!reportDir.exists()) // create reportPath if necessary
        QDir().mkdir(reportDir.absolutePath());

    Breakpad::CrashHandler::instance()->Init(reportDir.absolutePath());

    // start application
    MainWindow w;
    w.show();
    return a.exec();
}
