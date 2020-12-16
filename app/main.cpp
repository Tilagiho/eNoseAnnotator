#include "lib/QCrashHandler/src/qcrashhandler.h"
#include <QApplication>
#include "classes/controler.h"

#define CLI_CURVE_FIT_OPTION "--curve-fit"

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

    // parse launch arguments & load file in first arg (if it exists)
    QCommandLineParser parser;
    parser.setApplicationDescription("eNoseAnnotator");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", QCoreApplication::translate("main", "Measurement file (.csv) to open"));

    QCommandLineOption curveFitOption(QStringList() << "curve-fit",
            QCoreApplication::translate("main", "Fit curves to exposition"));
    parser.addOption(curveFitOption);

    parser.process(a);

    const QStringList args = parser.positionalArguments();
    bool curveFit = parser.isSet(curveFitOption);

    if (curveFit)
    {
        throw std::runtime_error("Requested curve fit with " + args[0].toStdString());
    }

    // timer to check arguments
    QTimer::singleShot(0, &c, &Controler::initialize);

    // start applivation
    c.getWindow()->show();
    return a.exec();
}
