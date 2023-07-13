#include "protocolcanbus.h"
#include <QDataStream>

ProtocolCanbus::ProtocolCanbus(const QString &connection)
    :ProtocolDriver(connection)
{
    _canDevice = nullptr;
}

ProtocolCanbus::~ProtocolCanbus()
{
    disconnectDevice();
}

bool ProtocolCanbus::connectDevice()
{
    //_canDevice = new QCanBusDevice();
    QString errorString;
    QList<QCanBusDeviceInfo> devs = QCanBus::instance()->availableDevices(QStringLiteral("socketcan"),&errorString);

    if(!errorString.isEmpty()){
        return false;
    }

    foreach (QCanBusDeviceInfo info, devs) {
        if(info.name().contains(connection())){
            _canDevice = QCanBus::instance()->createDevice(QStringLiteral("socketcan"),connection(),&errorString);
            if(errorString.isEmpty()){
                connect(_canDevice,&QCanBusDevice::errorOccurred,this,&ProtocolCanbus::handleError);
                connect(_canDevice,&QCanBusDevice::framesReceived,this,&ProtocolCanbus::handleRead);
                _canDevice->connectDevice();
                setState(CONNECTED);
                return true;
            }
        }
    }
    return false;
}

void ProtocolCanbus::disconnectDevice()
{
    if(_canDevice == nullptr){
        return;
    }
    _canDevice->disconnectDevice();
    _canDevice = nullptr;
    setState(DISCONNECTED);
}

QByteArray ProtocolCanbus::readFrame()
{
    QByteArray ret;
    QDataStream ds(&ret,QIODevice::ReadWrite);
    if(_receivedFrame.count() > 0){
        QCanBusFrame f = _receivedFrame.first();
        ds << f.frameId();
        //ds << f.frameType();
        ds << f.payload();
        _receivedFrame.removeFirst();
    }
    return ret;
}

bool ProtocolCanbus::writeFrame(const QByteArray &data)
{
    if(_canDevice != nullptr){
        QCanBusFrame frame;
        QByteArray d = data;
        QDataStream ds(&d, QIODevice::ReadOnly);
        quint32 fid;
        QByteArray payload;
        ds >> fid;
        ds >> payload;
        frame.setFrameId(fid);
        frame.setPayload(payload);

        _canDevice->writeFrame(frame);
    }

   return false;
}

void ProtocolCanbus::notifyRead()
{
    emit frameReceived();
}

void ProtocolCanbus::handleError()
{
    disconnectDevice();
}

void ProtocolCanbus::handleRead()
{
    QCanBusDevice *dev = (QCanBusDevice*)sender();

    qint64 frames = dev->framesAvailable();
    for(int i=0;i<frames;i++){
        QCanBusFrame f = dev->readFrame();
        _receivedFrame.append(f);
    }
    if(frames > 0){
        emit frameReceived();
    }
}

