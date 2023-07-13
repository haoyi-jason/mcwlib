#include "canbusprotocol.h"

CanbusProtocol::CanbusProtocol(CanBusDriver *driver)
{
    _driver = nullptr;
    _reports.append(new CAN_Report(0x116));
    _reports.append(new CAN_Report(0x126));
    _reports.append(new CAN_Report(0x216));
    _reports.append(new CAN_Report(0x226));

    setProtocolDriver(driver);

}

CanbusProtocol::~CanbusProtocol()
{
    if(_driver != nullptr){
        _driver->deleteLater();
    }
}

QString CanbusProtocol::connectionString() const
{
    return _connectionString;
}

void CanbusProtocol::setBusName(const QString &busName)
{

}
void CanbusProtocol::setConnectionString(const QString &connection)
{
    _connectionString = connection;
}

void CanbusProtocol::start()
{
    if(_driver != nullptr){
        if(_driver->state() == CanBusDriver::DISCONNECTED){
            _driver->connectDevice();
        }
        connect(_driver,&CanBusDriver::framesReceived,this,&CanbusProtocol::frameRec,Qt::UniqueConnection);
        //connect(_driver,&CanBusDriver::stateChanged,this,&CanbusProtocol::updateState);
    }

}

void CanbusProtocol::stop()
{

}

CanBusDriver *CanbusProtocol::driver() const
{
    return _driver;
}

void CanbusProtocol::setProtocolDriver(CanBusDriver *driver)
{
    if(_driver != nullptr){
        _driver->deleteLater();
    }

    _driver = driver;
}

bool CanbusProtocol::isConnected() const
{
    if(_driver == nullptr){
        return false;
    }
    return _driver->state() == CanBusDriver::CONNECTED;
}

bool CanbusProtocol::canWrite() const
{
    return !((_driver == nullptr));
}

bool CanbusProtocol::writeFrame(const QCanBusFrame &data)
{
    if(!canWrite()){
        return false;
    }

    _driver->writeFrame(data);
    return true;
}

void CanbusProtocol::frameRec()
{
    if(_driver == nullptr){
        return;
    }

    bool stop = false;
    while(!stop){
        QCanBusFrame data = _driver->readFrame();
        if(data.frameId() != 0x00){
            feedData(data);
        }
        else{
            stop = true;
        }

    }


}

void CanbusProtocol::feedData(QCanBusFrame frame)
{
    foreach (CAN_Report *r, _reports) {
        if(r->_DeviceId == frame.frameId()){
            QByteArray a = frame.payload();
            r->unpackData(a);
            emit driverUpdate(r->_DeviceId,r->_Enabled,r->_ErrorCode);
        }
    }
}


QString CanbusProtocol::summary() const
{
    QString msg;

    foreach (CAN_Report *r, _reports) {
        msg += QString("ID= %1, Enabled= %2, Error= %3\n").arg(r->_DeviceId,r->_Enabled,r->_ErrorCode);
    }
    return msg;
}











