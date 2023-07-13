#include "protocolserial.h"

ProtocolSerial::ProtocolSerial(const QString &connection)
    :ProtocolDriver(connection)
{
    _serialPort = nullptr;
}
ProtocolSerial::~ProtocolSerial()
{
    disconnectDevice();
}

bool ProtocolSerial::connectDevice()
{
    //QMutexLocker socketLocket(&_socketMutex);

    _serialPort = new QSerialPort();
    _serialPort->setPortName(connection());
    _serialPort->setBaudRate(QSerialPort::Baud115200);
    _serialPort->setParity(QSerialPort::NoParity);
    _serialPort->setDataBits(QSerialPort::Data8);
    _serialPort->setStopBits(QSerialPort::OneStop);

    if(_serialPort->open(QIODevice::ReadWrite)){
        setState(CONNECTED);
        connect(_serialPort,&QSerialPort::readyRead,this,&ProtocolSerial::handleRead);
    }
    setState(DISCONNECTED);

    return false;
}

void ProtocolSerial::disconnectDevice()
{
    if(_serialPort == nullptr){
        return;
    }

    _serialPort->close();
    _serialPort = nullptr;
    setState(DISCONNECTED);
}

QByteArray ProtocolSerial::readFrame()
{
    QByteArray ret = _receivedData;
    _receivedData.clear();
    return ret;
}

bool ProtocolSerial::writeFrame(const QByteArray &frame)
{
    if(_serialPort == nullptr){
        return false;
    }
    if(!_serialPort->isOpen()){
        return false;
    }

    _serialPort->write(frame);

    return true;
}

void ProtocolSerial::notifyRead()
{
    emit frameReceived();
}

void ProtocolSerial::handleRead()
{
    _receivedData.append(_serialPort->readAll());
    notifyRead();
}

void ProtocolSerial::handleError()
{
    disconnectDevice();
}
