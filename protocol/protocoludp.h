#ifndef PROTOCOLUDP_H
#define PROTOCOLUDP_H
#include "mcwlib.h"
#include "protocoldriver.h"

#include <QMutex>
#include <QSocketNotifier>
#include <QThread>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QHostAddress>
#include <QNetworkInterface>

class MCWLIB_EXPORT ProtocolSocketNotifierThread;

class ProtocolUDP: public ProtocolDriver
{
    Q_OBJECT
public:
    const int APCU_RX_PORT=26001;
    const int APCU_TX_PORT=26000;
    const int SIMULATOR_PORT=26003;
    ProtocolUDP(const QString &connection);
    ~ProtocolUDP() override;

public:
    bool connectDevice() override;
    void disconnectDevice() override;

    QByteArray readFrame() override;
    bool writeFrame(const QByteArray &frame) override;



private:
    int _protocolSocket;
    QMutex _socketMutex;
//    friend class ProtocolSocketNotifierThread;
//    ProtocolSocketNotifierThread *_readNotifier;
    QSocketNotifier *_errorNotifier;
    QUdpSocket *_socket;
    QByteArray _receivedData;
    QUdpSocket *_txSocket;
    QHostAddress _destAddr;

    void notifyRead();

    int _rxPort,_txPort;
    QString _ipAddr;

protected slots:
    void handleError();
    void handleRead();
};

//class ProtocolSocketNotifierThread:public QThread
//{
//    Q_OBJECT
//public:
//    ProtocolSocketNotifierThread(ProtocolDriver *driver);

//protected:
//    int _socket;
//    void run() override;
//    ProtocolUDP *_driver;
//};

#endif // PROTOCOLUDP_H
