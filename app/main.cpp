#include "widgets/mainwindow.h"

#include "lib/QCrashHandler/src/qcrashhandler.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("eNoseAnnotator");

    // setup breakpad crash handler:
    // save crash minidumps in reports
    QString reportPath = a.applicationDirPath() + "/crash reports";
    if (!QDir(reportPath).exists()) // create reportPath if necessary
        QDir().mkdir(reportPath);

    Breakpad::CrashHandler::instance()->Init(reportPath);

    // start application
    MainWindow w;
    w.show();
    return a.exec();
}
