#include "consolelogger.h"
#include <QTextStream>
#include <QFile>
#include <QByteArray>

ConsoleLogger::ConsoleLogger(QString connection, QObject *parent) : QObject(parent)
{
    _serialPort = nullptr;
    _connection = connection;
    _enableLog = false;
}

ConsoleLogger::~ConsoleLogger()
{
    disconnectDevice();
}

bool ConsoleLogger::connectDevice()
{
    _serialPort = new QSerialPort();
    _serialPort->setPortName(connection());
    _serialPort->setBaudRate(QSerialPort::Baud115200);
    _serialPort->setParity(QSerialPort::NoParity);
    _serialPort->setDataBits(QSerialPort::Data8);
    _serialPort->setStopBits(QSerialPort::OneStop);

    if(_serialPort->open(QIODevice::ReadWrite)){
        setState(CONNECTED);
        connect(_serialPort,&QSerialPort::readyRead,this,&ConsoleLogger::handleRead);
    }
    setState(DISCONNECTED);

    return false;
}

QString ConsoleLogger::connection() const
{
    return _connection;
}

void ConsoleLogger::disconnectDevice()
{
    if(_serialPort != nullptr){
        _serialPort->close();
        _serialPort->deleteLater();
        _serialPort = nullptr;
    }
    setState(DISCONNECTED);
}

bool ConsoleLogger::startLog(QString fileName)
{
    _logFileName = fileName;
//    _logFile = new QFile(fileName);
//    if(_logFile->open(QIODevice::WriteOnly | QIODevice::Append)){
//        return true;
//    }
    _enableLog = true;
    return true;
}

void ConsoleLogger::setState(STATE state)
{
    _state = state;
}

ConsoleLogger::STATE ConsoleLogger::state() const
{
    return _state;
}

void ConsoleLogger::stopLog()
{
    //_logFile->close();
    _enableLog = false;
}

void ConsoleLogger::handleRead()
{
    if(_enableLog){
        QSerialPort *p = static_cast<QSerialPort*>(sender());
        QFile _file(_logFileName);
        if(_file.open(QIODevice::WriteOnly | QIODevice::Append)){
            QTextStream stream(&_file);
            QByteArray data = p->readAll();
            stream << data;
            _file.flush();
            _file.close();
        }
    }
}
