#ifndef CANBUSPROTOCOL_H
#define CANBUSPROTOCOL_H

#include "mcwlib.h"
#include <QObject>
#include <QTimer>
#include <QDataStream>
#include "busdriver/qcanbusframe.h"

//#include "protocoldriver.h"
#include "busdriver/canbussocketcan.h"

// only 8-th byte data resolved
class MCWLIB_EXPORT CAN_Report:public QObject
{
    Q_OBJECT
public:
    enum ERROR_CODE{
        ERR_NOERR,
        ERR_OVERVOLTAGE,
        ERR_UNDERVOLTAGE,
        ERR_OVERCURRENT,
        ERR_OVERTEMMCU,
        ERR_OVERTEMMOTOR,
        ERR_LT2_ERROR,
        ERR_LT2_WARNING,
        ERR_FEEDBACK,
    };
    Q_ENUM(ERROR_CODE)
    CAN_Report(quint16 id){
        _DeviceId = id;
    }
    bool unpackData(QByteArray &data){
        if(data.size()>= 8){
            quint8 d8 = data.at(7);
            if((d8 & 0x80)){
                _Enabled = true;
            }
            else{
                _Enabled = false;
            }

            d8 = (d8 >> 4);
            _ErrorCode = (ERROR_CODE)(d8 & 0x7);
            return true;
        }
        return false;
    }

    quint16 _DeviceId;
    ERROR_CODE _ErrorCode;
    bool _Enabled;


};

class CanbusProtocol:public QObject
{
    Q_OBJECT
public:
    explicit CanbusProtocol(CanBusDriver *driver = nullptr);
    ~CanbusProtocol();

    CanBusDriver *driver() const;
    void setProtocolDriver(CanBusDriver *driver);
    bool isConnected() const;
    bool canWrite() const;
    bool writeFrame(const QCanBusFrame &data);

    QString connectionString() const;
    QString summary() const;

public slots:
    void setBusName(const QString &busName);
    void setConnectionString(const QString &connection);
    void start();
    void stop();

signals:
    void frameAvailable();
    void driverUpdate(quint32 id, quint8 state, quint8 error);

protected slots:
    void frameRec();

protected:
    QString _busName;
    CanBusDriver *_driver;
    QString _connectionString;

private:
    QList<CAN_Report*> _reports;
    void feedData(QCanBusFrame frame);


};

#endif // CANBUSPROTOCOL_H
