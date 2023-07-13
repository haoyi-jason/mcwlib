#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QMutex>

class AxisConfig;
class AD5764Config;
class SerialPortConfig;
class RcuConfig;
class SystemPath;
// singleton object
class Settings : public QObject
{
    Q_OBJECT
public:
    enum RCU_AXIS{
        AZ,
        EL
    };
    Q_ENUM(RCU_AXIS)

    QString getInstanceName() const;
    static Settings &instance();
    static void release();
    virtual ~Settings();
    void LoadConfig();
    void StoreConfig();
    QString logPath() ;
    // user function

    static AxisConfig *config(RCU_AXIS axis);
    static AD5764Config *dacConfig(int channel);
    static SerialPortConfig *serialConfig(int channel);
    static void setSerialPortConfig(SerialPortConfig *cfg,int channel);
    static SerialPortConfig *canConfig(int channel);
    static void setCanConfig(SerialPortConfig *cfg,int channel);
    static RcuConfig *rcuConfig();

    static void Info(const QString &msg);
    static void Warning(const QString &msg);
    static void Debug(const QString &msg);

    static int SerialPortCount() ;
    static int CanPortCount() ;
    static QStringList supportedSerialPorts() ;
    static void startCanBus();

protected:
    Settings(QObject *parent = nullptr);
    Settings(const Settings &other);
    Settings& operator=(const Settings &other);

    static QScopedPointer<Settings> _instance;
    QMutex _mutex;
    QList<AxisConfig*> _axisConfig;
    QList<AD5764Config*> _dacConfig;
    QList<SerialPortConfig*> _serialPortConfig;
    QList<SerialPortConfig*> _canPortConfig;
    RcuConfig *_rcuConfig;
    QString _logPath;

signals:

public slots:
};

class SystemPath{
public:
    QString LogFolder;
    QString TraceFolder;
};

class RcuConfig
{
public:
    enum RCU_INTERFACE{
        RCU_USE_UDP,
        RCU_USE_422
    };
    RCU_INTERFACE Interface;
    QString ConnectString;
    int tx_port, rx_port;
    ulong baudrate;
    bool valid_checksum;
    int reportInterval;
    int stateReportInterval;
};

class AxisConfig
{
public:
//    AxisConfig();

//private:
    double pos_limit_low, pos_limit_high;
    double speed_low, speed_high;
    double output_low, output_high;

    double skp,ski,skd;
    double pkp,pki,pkd;

};

class AD5764Config
{
public:
    enum CORSE_GAIN{
        CG_B10V,
        CG_B10264V,
        CG_B105263V
    };
    //Q_ENUM(CORSE_GAIN)
    CORSE_GAIN CorseGain;
    quint8 FineGain;
    quint8 Offset;
};

class SerialPortConfig
{
public:
    QString connection;
    ulong baudrate;
    QString parity;
    quint8 databits;
    QString stopbits;
    QString flowControl;
    QString mode;

    void operator=(SerialPortConfig &b){
        this->connection = b.connection;
        this->baudrate = b.baudrate;
        this->parity = b.parity;
        this->databits = b.databits;
        this->stopbits = b.stopbits;
        this->flowControl = b.flowControl;
        this->mode = b.mode;
    }
};

#endif // SETTINGS_H
