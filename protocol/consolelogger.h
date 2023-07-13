#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "mcwlib.h"
#include <QObject>
#include <QSerialPort>

class QFile;
class QTextStream;
class MCWLIB_EXPORT ConsoleLogger : public QObject
{
    Q_OBJECT
public:
    enum STATE{
        DISCONNECTED,
        CONNECTED,
        ERROR
    };
    explicit ConsoleLogger(QString connection,QObject *parent = nullptr);
    ~ConsoleLogger();
    bool connectDevice();
    void disconnectDevice();

    QString connection() const;
    STATE state() const;
    bool startLog(QString fileName);
    void stopLog();
signals:
    void lineReceived(int);
private slots:
    void handleRead();

private:
    void setState(STATE state);
private:
    QString _connection;
    QSerialPort *_serialPort;
    quint32 _lineReceived;
    STATE _state;

    QFile *_logFile;
    QString _logFileName;
    bool _enableLog;

};

#endif // CONSOLELOGGER_H
