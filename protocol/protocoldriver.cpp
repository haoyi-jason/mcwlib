#include "protocoldriver.h"

ProtocolDriver::ProtocolDriver(QString connection)
    :_connection(connection)
{
    _state = DISCONNECTED;
    _connectionType = CONN_UNKNOW;
}

ProtocolDriver::State ProtocolDriver::state() const
{
    return _state;
}

bool ProtocolDriver::connectDevice()
{

}

void ProtocolDriver::disconnectDevice()
{

}

QByteArray ProtocolDriver::readFrame()
{
    return QByteArray();
}

bool ProtocolDriver::writeFrame(const QByteArray &frame)
{
    Q_UNUSED(frame);
    return false;
}

void ProtocolDriver::setState(const State &state)
{
    bool stateChange = (_state != state);
    _state = state;
    if(stateChange){
        emit stateChanged(_state);
    }
}

const QString ProtocolDriver::connection() const
{
    return _connection;
}

void ProtocolDriver::setConnection(const QString &connection)
{
    _connection = connection;
}


