#include "ProcessObserver.h"
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QDir>
#include <QCoreApplication>
#include <Logger.h>
#include <common/constants.h>

#define MAX_MB_SIZE 300

ProcessObserver::ProcessObserver(const QString& path, const QStringList &arguments)
    : m_processPath(path)
    , m_arguments(arguments)
    , m_wasStarted(false)
{
    LOG_INFO("Process path: " + m_processPath);

    m_checkTimer.start(2000);

    connect( &m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessObserver::onFinished);
    connect( &m_process, &QProcess::started, this, &ProcessObserver::onStarted);
    connect( &m_checkTimer, &QTimer::timeout, this, &ProcessObserver::onCheckProcess);
}

ProcessObserver::~ProcessObserver()
{
}

void ProcessObserver::start()
{
    if(!m_wasStarted) {
        LOG_INFO(QString("Process was started at %1").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss")));
	}
	else {
        LOG_INFO(QString("Process was restarted at %1").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss")));
	}

    QString command = QDir::toNativeSeparators( m_processPath );
    m_process.start(command, m_arguments);
    m_elapsedTime.restart();
    m_wasStarted = true;
}

void ProcessObserver::saveAndExit()
{
	exit(0);
}

void ProcessObserver::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(m_wasStarted && exitStatus == QProcess::CrashExit && m_elapsedTime.elapsed() < 30000) {
        LOG_WARNING(QString("Fast crash! Process was crashed. Exit code: %1").arg(exitCode));
        saveAndExit();
	}

    if( exitStatus == QProcess::NormalExit ) {
        if ( exitCode == 66 ) {
            LOG_INFO(QString("Process was terminated by memory leak. Exit code: %1. Restaring").arg(exitCode));
            QTimer::singleShot( 3000, this, SLOT(start()) );
        } else {
            LOG_INFO(QString("Process was successfuly finished. Exit code: %1").arg(exitCode));
            saveAndExit();
        }
	}
	else {
        LOG_WARNING(QString("Process was crashed. Exit code: %1. Restarting...").arg(exitCode));
        QTimer::singleShot( 500, this, SLOT(start()) );
    }
}

void ProcessObserver::onStarted()
{
}

void ProcessObserver::onCheckProcess()
{
    QSystemSemaphore sema(GLOBAL_APP_NAME + " unic sema name", 1);
	// Проверка на единственность запущенного экземпляра
	sema.acquire();
	{
        QSharedMemory shmem(GLOBAL_APP_NAME + " unic name");
		shmem.attach();
	}

    QSharedMemory shmem(GLOBAL_APP_NAME + " unic name");
	bool isRunning = shmem.attach();

	sema.release();

    if( isRunning) {
        LOG_WARNING("Can not find process. Exiting...");
        saveAndExit();
    }
}
