#include "protocoludp.h"

ProtocolUDP::ProtocolUDP(const QString &connection)
    :ProtocolDriver(connection)
{
    _protocolSocket = -1;
    //_readNotifier = nullptr;
    _errorNotifier = nullptr;
    _destAddr = QHostAddress::Broadcast;
//    _destAddr = QHostAddress("192.168.1.255");

    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface nic, ifaces) {
        if(nic.name().contains("eth0")){
            QList<QNetworkAddressEntry> addrs = nic.addressEntries();
            foreach(QNetworkAddressEntry a, addrs){
                if(a.broadcast().toString() != ""){
//                    qDebug()<<"Broadcast:"<<a.broadcast();
                    _destAddr = a.broadcast();
                    emit setLog(QString("Set Broadcast ip to:%1").arg(_destAddr.toString()));
                }
            }
        }
    }

    // connection : ip,rxport,txport
    QStringList sl = connection.split(",");
    if(sl.size() == 1){
        _ipAddr = sl[0];
        _rxPort = APCU_RX_PORT;
        _txPort = APCU_TX_PORT;
    }
    else if(sl.size() == 2){
        _ipAddr = sl[0];
        _rxPort = sl[1].toInt();
        _txPort = APCU_TX_PORT;
    }
    else if(sl.size() == 3){
        _ipAddr = sl[0];
        _rxPort = sl[1].toInt();
        _txPort = sl[2].toInt();
    }
    else{
        _ipAddr = "localhost";
        _rxPort = APCU_RX_PORT;
        _txPort = APCU_TX_PORT;
    }
    //qDebug()<<ifaces;
}

ProtocolUDP::~ProtocolUDP()
{
    disconnectDevice();
}

bool ProtocolUDP::connectDevice()
{
    QMutexLocker socketLocket(&_socketMutex);

    _socket = new QUdpSocket(this);
//    _socket->bind(QHostAddress("255.255.255.255"),APCU_RX_PORT,QUdpSocket::ReuseAddressHint);
//    _socket->bind(APCU_RX_PORT, QUdpSocket::ReuseAddressHint);
    _socket->bind(_rxPort, QUdpSocket::ReuseAddressHint);

    //_socket->bind(QHostAddress::LocalHost, APCU_RX_PORT, QUdpSocket::ReuseAddressHint);
    //_socket->setSocketOption(socketo);

//    _readNotifier = new ProtocolSocketNotifierThread(this);
//    _readNotifier->start();
    connect(_socket,&QUdpSocket::readyRead,this,&ProtocolUDP::handleRead);


    //qDebug()<<"UDP Bind Error:"<<_socket->errorString();
    emit setLog(QString("UDP Bind Error: %1").arg(_socket->errorString()));

    setState(CONNECTED);

    emit setLog(QString("Bind UDP Socket to %1").arg(_socket->localAddress().toString()));
    qDebug()<<(QString("Bind UDP Socket to %1").arg(_socket->localAddress().toString()));

//    _txSocket = new QUdpSocket(this);
//    _txSocket->bind(APCU_TX_PORT,QUdpSocket::ReuseAddressHint);
    return true;
}

void ProtocolUDP::disconnectDevice()
{
    QMutexLocker socketLocker(&_socketMutex);
    if(_socket == nullptr){
        return;
    }

    //_readNotifier->terminate();
    _socket->close();
    _socket = nullptr;
    setState(DISCONNECTED);
    //_readNotifier->deleteLater();

//    _txSocket->close();
//    _txSocket = nullptr;
}

QByteArray ProtocolUDP::readFrame()
{
    QMutexLocker socketLocket(&_socketMutex);
//    QByteArray data = _socket->readAll();
    QByteArray data = _receivedData;
    _receivedData.clear();
    return data;
}

bool ProtocolUDP::writeFrame(const QByteArray &frame)
{
    //QMutexLocker socketLocker(&_socketMutex);
//    qint64 ret= _socket->writeDatagram(frame,QHostAddress::Any,APCU_TX_PORT);
    qint64 ret= _socket->writeDatagram(frame,_destAddr,_txPort);
    //qDebug()<<Q_FUNC_INFO <<" byte send:"<<ret <<" at port:"<<APCU_TX_PORT;
    //QUdpSocket *s = new QUdpSocket();
    //s->writeDatagram(frame,QHostAddress::Broadcast,APCU_TX_PORT);

    return true;
}

void ProtocolUDP::notifyRead()
{
    emit frameReceived();
}

void ProtocolUDP::handleRead()
{
    //qDebug()<<Q_FUNC_INFO;
    while(_socket->hasPendingDatagrams()){
        QNetworkDatagram datagram = _socket->receiveDatagram();

        _receivedData.append(datagram.data());
        //qDebug()<<"Address:"<<datagram.senderAddress().toString();
        _destAddr = datagram.senderAddress();
        notifyRead();
    }
}

void ProtocolUDP::handleError()
{
    disconnectDevice();
}
