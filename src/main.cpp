#include <QGuiApplication>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QTranslator>
#include <QIcon>
#include <QFontDatabase>
#include "cryptoshloma.h"
#include "common/constants.h"
#include "common/settings_manager.h"
#include <Logger.h>
#include <QDir>
#include <QStandardPaths>
#include "include/version.hpp"
#include "common/stacktrace.h"
#include <init_logger.h>

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0)) && defined(QT_STATIC)
#include <QtPlugin>
Q_IMPORT_PLUGIN(QtQuick2Plugin)
Q_IMPORT_PLUGIN(QtQuickControls2Plugin)
Q_IMPORT_PLUGIN(QtQuickControls2MaterialStylePlugin)
Q_IMPORT_PLUGIN(QtQuick2WindowPlugin)
Q_IMPORT_PLUGIN(QtQuickTemplates2Plugin)
#endif

#ifdef BREAKPAD_ENABLED
#ifdef Q_OS_WIN
#include "client/windows/handler/exception_handler.h"
#else
#include "client/linux/handler/exception_handler.h"
#endif
static bool dumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
                         void* context,
                         bool succeeded) {
    Q_UNUSED(context)
    LOG_ERROR("Dump path: %s\n", descriptor.path());
    return succeeded;
}
#endif

void initSettings()
{
    QFile cfgDefaults{":/common/resources/default.cfg"};
    if ( !cfgDefaults.open(QIODevice::ReadOnly) )
        LOG_ERROR(cfgDefaults.errorString());
    SettingsManager::setDefaults(cfgDefaults.readAll());
    cfgDefaults.close();
}

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

#ifdef BREAKPAD_ENABLED
#ifndef QT_NO_DEBUG
    const QString crashDumpDirName{app.applicationDirPath() + QStringLiteral("/crash_dump")};
#else
    const QString crashDumpDirName{QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString(QStringLiteral("/.%1/crash_dump")).arg(GLOBAL_APP_NAME)};
#endif
    google_breakpad::MinidumpDescriptor descriptor(crashDumpDirName.toStdString());
    google_breakpad::ExceptionHandler eh(descriptor, nullptr, dumpCallback, nullptr, true, -1);
    QDir crashDir{crashDumpDirName};
    if ( !crashDir.exists() )
        crashDir.mkpath(crashDumpDirName);
#else
    installSignalHandlers();
#endif
    app.setOrganizationName(GLOBAL_APP_NAME);
    app.setApplicationName(GLOBAL_APP_NAME);
    app.setApplicationVersion(cryptoShlomaVersionStr());

    QCommandLineParser parser;
    parser.setApplicationDescription("Help information");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   QCoreApplication::translate("Debug", "Enable debug mode."));
    parser.addOption(debugOption);

    QCommandLineOption traceOption(QStringList() << "t" << "trace",
                                   QCoreApplication::translate("Trace", "Enable trace logging."));
    parser.addOption(traceOption);

    parser.process(app);
    bool isDebugModeEnabled = parser.isSet(debugOption);

    Logger::LogLevel level;
    if ( isDebugModeEnabled )
        level = Logger::Debug;
    else
        level = Logger::Warning;

    if ( parser.isSet(traceOption) )
        level = Logger::Trace;
    app.setWindowIcon(QIcon(QStringLiteral(":/ui/icons/resources/ui_icons/rabbi.svg")));

    initLogger(&app, level);
    initSettings();
    const int id = QFontDatabase::addApplicationFont(QStringLiteral(":/ui/fonts/resources/fonts/Montserrat-Regular.ttf"));
    if ( id > -1 ) {
        const QString family = QFontDatabase::applicationFontFamilies(id).first();
        const QFont globalFont{family};
        app.setFont(globalFont);
    } else
        LOG_WARNING("default fon from resources no be loaded");
//    QTranslator qtTranslator;
//    qtTranslator.load("qt_" + QLocale::system().name(),
//    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
//    app.installTranslator(&qtTranslator);

    QQuickStyle::setStyle("Material");

    CryptoShloma *bot = new CryptoShloma(&app);
    QObject::connect(&app, &QGuiApplication::aboutToQuit, bot, &CryptoShloma::deleteLater, Qt::QueuedConnection);
    if ( !bot->init() )
        return -1;

    return app.exec();
}

