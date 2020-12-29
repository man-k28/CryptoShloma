#ifndef INIT_LOGGER_H
#define INIT_LOGGER_H
#include <QCoreApplication>
#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>
#include <QDir>
#include <QStandardPaths>
#include <common/constants.h>

void initLogger(QCoreApplication *app, const Logger::LogLevel level)
{
#ifndef QT_NO_DEBUG
    const QString logDirName{app->applicationDirPath() + QStringLiteral("/logs")};
#else
    const QString logDirName{QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString(QStringLiteral("/.%1/logs")).arg(GLOBAL_APP_NAME)};
#endif
    QDir logDir{logDirName};
    if ( !logDir.exists() )
        logDir.mkpath(logDirName);

    const QString logFormat{QStringLiteral("[%{time}{dd-MM-yyyy HH:mm:ss}] [%{type}] <%{function}:%{line}> %{message}\n")};
    const QString &fileFormat = QString("%1/%2.log")
                                    .arg(logDirName)
                                    .arg(app->applicationName());
#ifndef QT_NO_DEBUG
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    consoleAppender->setFormat(logFormat);
    consoleAppender->setDetailsLevel(level);
    cuteLogger->registerAppender(consoleAppender);
#endif
    RollingFileAppender *rollingFileAppender = new RollingFileAppender(fileFormat);
    rollingFileAppender->setFormat(logFormat);
    rollingFileAppender->setDetailsLevel(level);
    rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);
    cuteLogger->registerAppender(rollingFileAppender);
}
#endif // INIT_LOGGER_H
