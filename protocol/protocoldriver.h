#ifndef PROTOCOLDRIVER_H
#define PROTOCOLDRIVER_H

#include "mcwlib.h"
#include <QObject>

class MCWLIB_EXPORT ProtocolDriver:public QObject
{
    Q_OBJECT
public:
    ProtocolDriver(QString connection);

    const QString connection() const;

    enum State{
        DISCONNECTED,
        CONNECTED,
        ERROR
    };
    Q_ENUM(State)

    enum ConnectionType{
        CONN_UNKNOW,
        CONN_RS422,
        CONN_CANBUS,
        CONN_UDP
    };
    Q_ENUM(ConnectionType)

    ProtocolDriver::State state() const;

    virtual bool connectDevice();
    virtual void disconnectDevice();

    virtual QByteArray readFrame();
    virtual bool writeFrame(const QByteArray &frame);
    void setConnection(const QString &connection);

signals:
    void frameReceived();
    void stateChanged(State state);
    void setLog(QString log);

protected:
    QString _connection;
    void setState(const State &state);

private:
    State _state;
    ConnectionType _connectionType;

};

#endif // PROTOCOLDRIVER_H
