#include <QCoreApplication>
#include "ProcessObserver.h"
#include <Logger.h>
#include <common/constants.h>
#include <init_logger.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    app.setApplicationName("Launcher");
    app.setOrganizationName(GLOBAL_APP_NAME);

    initLogger(&app, Logger::Debug);

    ProcessObserver observer(app.applicationDirPath() + "/" + GLOBAL_APP_NAME, app.arguments());
    observer.start();
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &observer, &ProcessObserver::deleteLater);
    return app.exec();
}
