#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QCommandLineParser>

#include "frame.h"
#include "interfaces.h"
#include "logmanager.h"

#include <libdui/dapplication.h>
#include <libdui/dthememanager.h>

DUI_USE_NAMESPACE

QString getQssFromFile(const QString &name)
{
    QString qss = "";

    QFile f(name);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qss = f.readAll();
        f.close();
    }

    return qss;
}

void onThemeChange(const QString &theme)
{
    QString fileName = QString(":/resources/qss/%1.qss").arg(theme);

    qApp->setStyleSheet(getQssFromFile(fileName));
}

int main(int argv, char *args[])
{
    DApplication app(argv, args);
    app.setOrganizationName("deepin");
    app.setApplicationName("DDE Control Center");
    app.setApplicationVersion("3.0");

    // take care of command line options
    QCommandLineOption showOption(QStringList() << "s" << "show", "show control center(hide for default).");
    QCommandLineParser parser;
    parser.setApplicationDescription("DDE Control Center");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(showOption);
    parser.addPositionalArgument("module", "the module's id of which to be shown.");
    parser.process(app);

    QStringList positionalArgs = parser.positionalArguments();

    // initialize logging
    LogManager::instance()->debug_log_console_on();

    Frame frame;
#ifndef QT_DEBUG
    if (parser.isSet(showOption))
#else
    frame.show();
#endif

        if (!positionalArgs.isEmpty()) {
            frame.selectModule(positionalArgs.at(0));
        }

    DBusControlCenter adaptor(&frame);
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService("com.deepin.dde.ControlCenter") ||
            !conn.registerObject("/com/deepin/dde/ControlCenter", &frame))
#ifndef QT_DEBUG
        return -1;
#else
        qWarning() << "d-bus service already registered!";
#endif

    // setup theme manager
    DThemeManager *manager = DThemeManager::instance();
    QObject::connect(manager, &DThemeManager::themeChanged, [ = ](QString theme) {
        onThemeChange(theme);
    });

    manager->setTheme("dark");
    onThemeChange("dark");

    return app.exec();
}
