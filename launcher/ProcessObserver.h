#ifndef PROCESSOBSERVER_H
#define PROCESSOBSERVER_H

#include <QTime>
#include <QProcess>
#include <QTimer>

class ProcessObserver : public QObject
{
	Q_OBJECT
public:
    explicit ProcessObserver(const QString& path, const QStringList& arguments);
	~ProcessObserver();

public slots:
	void start();
private:
	void saveAndExit();
private slots:
	void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onStarted();
	void onCheckProcess();
private:
    QProcess  m_process{};                       /// < Процесс, который будем запускать
    QString   m_processPath;
    QStringList   m_arguments;
    QTime     m_elapsedTime{};                   /// < Время работы процесса
    bool      m_wasStarted;                      /// < Флаг того, что процесс был запущен
    QTimer    m_checkTimer{};                    /// < Таймер проверки наличия процесса
};

#endif // PROCESSOBSERVER_H
