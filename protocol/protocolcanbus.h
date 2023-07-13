#ifndef PROTOCOLCANBUS_H
#define PROTOCOLCANBUS_H

#include "mcwlib.h"
#include <QMutex>
#include <QtSerialPort/QSerialPort>
#include <QThread>
#include <QtSerialBus/QCanBus>
#include <QtSerialBus/QCanBusDevice>
#include <QtSerialBus/QCanBusDeviceInfo>
//#include <QtSerialBus/QCanBusFrame>
#include "busdriver/qcanbusframe.h"
#include "protocoldriver.h"

class MCWLIB_EXPORT ProtocolCanbus:public ProtocolDriver
{
    Q_OBJECT
public:
    ProtocolCanbus(const QString &connection);
    ~ProtocolCanbus();

    bool connectDevice() override;
    void disconnectDevice() override;

    QByteArray readFrame() override;
    bool writeFrame(const QByteArray &frame) ;

private:
    QCanBusDevice *_canDevice;
    QByteArray _receivedData;
    QList<QCanBusFrame> _receivedFrame;
    void notifyRead();

private slots:
    void handleError();
    void handleRead();

};

#endif // PROTOCOLCANBUS_H
