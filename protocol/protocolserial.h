#ifndef PROTOCOLSERIAL_H
#define PROTOCOLSERIAL_H
#include "mcwlib.h"
#include "protocoldriver.h"

#include <QMutex>
#include <QtSerialPort/QSerialPort>
#include <QThread>

class MCWLIB_EXPORT ProtocolSerial: public ProtocolDriver
{
    Q_OBJECT
public:
    const int BAUDRATE = 115200;

    ProtocolSerial(const QString &connection);
    ~ProtocolSerial() override;

public:
    bool connectDevice() override;
    void disconnectDevice() override;

    QByteArray readFrame() override;
    bool writeFrame(const QByteArray &frame) override;

private:
    QSerialPort *_serialPort;
    QByteArray _receivedData;

    void notifyRead();

protected slots:
    void handleError();
    void handleRead();
};

#endif // PROTOCOLSERIAL_H
