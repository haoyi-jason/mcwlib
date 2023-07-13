#ifndef RCUPROTOCOL_H
#define RCUPROTOCOL_H
#include "mcwlib.h"
#include <QObject>
#include "protocoldriver.h"
#include <QTimer>
#include <QDataStream>


class MCWLIB_EXPORT RCU_report
{
public:
    RCU_report(){
        header = 0x5A;
        cmd_type = 0x0;
        length = 0x0;
        checksum = 0x0;
    }
    quint8 header;
    quint8 cmd_type;
    quint8 length;
    quint8 checksum;
    virtual quint8 calculateChecksum(){
        quint8 sum = 0x0;
        sum = cmd_type + length;
        for(int i=0;i<_packet.count();i++){
            sum += _packet.at(i);
        }
        return sum;
    }
    virtual QByteArray packetData()
    {
        QByteArray ret;
        QDataStream ds(&ret,QIODevice::ReadWrite);
        checksum = calculateChecksum();
        ds << header;
        ds << cmd_type;
        ds << length;
        for(int i=0;i<_packet.size();i++){
            quint8 c = _packet.at(i);
            ds << c;
        }

        ds << checksum;
        return ret;
    }
    virtual bool unpackData(QByteArray &data)
    {
        QDataStream ds(&data,QIODevice::ReadOnly);
        ds >> header;
        ds >> cmd_type;
        ds >> length;
        _packet.resize(length);
        _packet = data.mid(3,length);
        //ds >> _packet;
        checksum = data.at(data.count()-1);

        quint8 sum = calculateChecksum();
        return (sum == checksum);
    }
protected:
    QByteArray _packet;
};

class R2A_report:public RCU_report
{
public:
    R2A_report():RCU_report()
    {
        cmd_type = 0x27;
        length = 0x10;
        AZ_BW = EL_BW = 1;
        CONTROL_CMD = 0x0;
        Predict_AZ = Predict_EL = 0x0;
        Flt_Alpha = Flt_Dt = 0;
        Stow_AZ = Stow_EL = Stow_Control = ANT_STOW = 0;

    }

    quint8 calculateChecksum() override
    {
        // call parent
        return RCU_report::calculateChecksum();
    }

    QByteArray packetData() override
    {
        // fill _packet struct by currne tdata
        _packet.resize(0x14);
        _packet.fill(0x0);
        QDataStream ds(&_packet,QIODevice::ReadWrite);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << AZ_BW;
        ds << EL_BW;
        ds << CONTROL_CMD;
        ds << Predict_AZ;
        ds << Predict_EL;
        ds << Flt_Alpha;
        ds << Flt_Dt;
        ds << Stow_AZ;
        ds << Stow_EL;
        ds << Stow_Control;
        ds << ANT_STOW;
        return RCU_report::packetData();
    }

    bool unpackData(QByteArray &data) override
    {
        bool valid = RCU_report::unpackData(data);

        QDataStream ds(&_packet, QIODevice::ReadOnly);
        ds.setByteOrder(QDataStream::LittleEndian);

        ds >> AZ_BW;
        ds >> EL_BW;
        ds >> CONTROL_CMD;
        ds >> Predict_AZ;
        ds >> Predict_EL;
        ds >> Flt_Alpha;
        ds >> Flt_Dt;
        ds >> Stow_AZ;
        ds >> Stow_EL;
        ds >> Stow_Control;
        ds >> ANT_STOW;

        return valid;
    }

    // data
    quint8 AZ_BW, EL_BW, CONTROL_CMD;
    quint16 Predict_AZ, Predict_EL;
    quint8 Flt_Alpha, Flt_Dt;
    quint8 Stow_AZ, Stow_EL, Stow_Control, ANT_STOW;
};

class R2A_report_joystick:public RCU_report
{
public:
    R2A_report_joystick():RCU_report()
    {
        cmd_type = 0x40;
        length = 0x10;
    }

    quint8 calculateChecksum() override
    {
        // call parent
        return RCU_report::calculateChecksum();
    }

    QByteArray packetData() override
    {
        // fill _packet struct by currne tdata
        _packet.resize(length);
        _packet.fill(0x0);
        QDataStream ds(&_packet,QIODevice::ReadWrite);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << Velocity_AZ;
        ds << Velocity_EL;
        return RCU_report::packetData();
    }

    bool unpackData(QByteArray &data) override
    {
        bool valid = RCU_report::unpackData(data);

        QDataStream ds(&_packet, QIODevice::ReadOnly);
        ds.setByteOrder(QDataStream::LittleEndian);

        ds >> Velocity_AZ;
        ds >> Velocity_EL;
        return valid;
    }

    // data
    qint16 Velocity_AZ, Velocity_EL;
};

class A2R_State:public RCU_report
{
public:
    explicit A2R_State():RCU_report()
    {
        cmd_type = 0x29;
        length = 0x11;
        INS = 0x00;
        APDU_AZ1 = 0x00;
        APDU_AZ2 = 0x00;
        APDU_EL1 = 0x00;
        APDU_EL2 = 0x00;
        AZ_STOW = 0x00;
        EL_STOW = 0x00;
        ANT_STOW = 0x00;
    }

    quint8 calculateChecksum() override
    {

        // call parent
        return RCU_report::calculateChecksum();
    }
    QByteArray packetData() override
    {
        // fill _packet struct by currne tdata
        _packet.resize(length);
        _packet.fill(0x0);
        QDataStream ds(&_packet,QIODevice::ReadWrite);
        ds << INS;
        ds << APDU_AZ1;
        ds << APDU_AZ2;
        ds << APDU_EL1;
        ds << APDU_EL2;
        ds << AZ_STOW;
        ds << EL_STOW;
        ds << ANT_STOW;
        return RCU_report::packetData();
    }

    bool unpackData(QByteArray &data) override
    {
        bool valid = RCU_report::unpackData(data);

        QDataStream ds(&_packet, QIODevice::ReadOnly);
        ds >> INS;
        ds >> APDU_AZ1;
        ds >> APDU_AZ2;
        ds >> APDU_EL1;
        ds >> APDU_EL2;
        ds >> AZ_STOW;
        ds >> EL_STOW;
        ds >> ANT_STOW;
        return valid;
    }


    // data
    quint8 INS;
    quint8 APDU_AZ1, APDU_AZ2;
    quint8 APDU_EL1, APDU_EL2;
    quint8 AZ_STOW, EL_STOW, ANT_STOW;


};

class A2R_Report:public RCU_report
{
public:
    A2R_Report():RCU_report()
    {
        cmd_type = 0x30;
        length = 0x11;
//        AP_AZ = (quint16)((222./360.)*65536);
//        AP_EL = (quint16)((111./360.)*65536);
        AP_AZ = 0;
        AP_EL = 0;

    }

    quint8 calculateChecksum() override
    {
        // fill _packet struct by currne tdata
        _packet.resize(length);
        _packet.fill(0x0);
        QDataStream ds(&_packet,QIODevice::ReadWrite);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << AP_AZ;
        ds << AP_EL;
        // call parent
        return RCU_report::calculateChecksum();
    }

    // data
    quint16 AP_AZ,AP_EL;


};


class RcuProtocol : public QObject
{
    Q_OBJECT
public:
    const int PACKET_SIZE=20;
    enum BANDWIDTH{
        BW_UNDEFINE,
        BW_0HZ,
        BW_1HZ,
        BW_2HZ,
        BW_4HZ,
        BW_6HZ,
    };
    Q_ENUM(BANDWIDTH)

    enum CONTROL_STATE{
        STATE_OFF,
        STATE_ON
    };
    Q_ENUM(CONTROL_STATE)

    enum RCU_FUNCTIONS{
        AZ_BW,
        EL_BW,
        SELF_TEST,
        RECORD,
        AZ_STOW,
        EL_STOW,
        STOW_CONTROL,
        ANT_STOW
    };
    Q_ENUM(RCU_FUNCTIONS)

    enum RCU_AXIS{
        RCU_AZ,
        RCU_EL,
        RCU_AZS,
        RCU_ELS,
    };
    Q_ENUM(RCU_AXIS)

    enum FILTERS{
        FILTER_ALPHA,
        FILTER_DT
    };
    Q_ENUM(FILTERS)


    explicit RcuProtocol(ProtocolDriver *pDriver = nullptr);
    ~RcuProtocol();

    QString busName() const;

    ProtocolDriver *driver() const;
    void setProtocolDriver(ProtocolDriver *driver);
    bool isConnected() const;
    bool canWrite() const;
    bool writeFrame(const QByteArray &data);

    QString connectionString() const;

    static QStringList supportedBandwidth();
    QString summary() const;

    int azbw() const;
    int elbw() const;
    bool selfTest() const;
    bool record() const;
    bool stow_az() const;
    bool stow_el() const;
    bool stow_ctrl() const;
    bool ant_stow() const;
    quint16 predictAZ() const;
    quint16 predictEL() const;
    quint16 filterAlpha() const;
    quint16 filterDt() const;

    void set_az_bw(int bw);
    void set_el_bw(int bw);
    void set_selftest(bool set);
    void set_record(bool set);
    void set_stow_az(bool set);
    void set_stow_el(bool set);
    void set_stow_ctrl(bool set);
    void set_ant_stow(bool set);
    void set_predict_az(quint16 value);
    void set_predict_el(quint16 value);
    void set_predict_az_rpm(quint16 value);
    void set_predict_el_rpm(quint16 value);
    void set_az_m1(bool set);
    void set_az_m2(bool set);
    void set_el_m1(bool set);
    void set_el_m2(bool set);

    A2R_Report *a2rReport() const;
    A2R_State *a2rState() const;

public slots:
    void setBusName(const QString &busName);
    void setConnectionString(const QString &connection);
    void start();
    void stop();
    void valid_checksum(bool on);
    void set_report_interval(int ms);
    void set_state_report_interval(int ms);
    void joystickMode(bool state);
    void hostMode(bool state);
    void setSimulatorData(QByteArray &data);

signals:
    void frameAvailable();
    void connectedChanged(bool);
    void busNameChanged(const QString &);
    // rcu related signals
    void state_changed(RCU_FUNCTIONS, CONTROL_STATE &);
    void bandwidth_changed(RCU_AXIS, BANDWIDTH &);
    void predict_changed(RCU_AXIS, quint16 &);
    void filter_changed(FILTERS, double &);
    void rcuReady(bool);

protected slots:
    void frameRec();
    void notifyForNewFrames();
    void updateState();
    void feedData(QByteArray &data);
    void handleTimeout();

protected:
    QString _busName;
    ProtocolDriver *_driver;
    QString _connectionString;
    QByteArray _receivedData;

private:
    void resetCounter();

private:
    BANDWIDTH _azbw, _elbw;
    CONTROL_STATE _self_test;
    CONTROL_STATE _record;
    CONTROL_STATE _stow_az,_stow_el;
    CONTROL_STATE _stow_control;
    CONTROL_STATE _ant_stow;
    quint16 _predict_az_raw, _predict_el_raw;
    quint16 _predict_az_rpm_raw, _predict_el_rpm_raw;
    quint16 _flt_alpha_raw, _flt_dt_raw;


    double _predict_az, _predict_el;
    double _flt_alpha,_flt_dt;
    bool _validChecksum;
    int _reportInterval;
    int _stateReportInterval;
    QTimer *_reportTimer;
    QTimer *_stateReportTimer;

    R2A_report _r2a_report;
    A2R_Report *_a2r_report;
    A2R_State *_a2r_state;
    R2A_report_joystick _r2a_report_joystick;
    quint8  _lastReportId;

    bool _joystickMode;
    bool _rcuHostMode;

    QByteArray _r2aData;

    quint32 _rcvCounter;
    bool _sendReadySignal;
};


#endif // RCUPROTOCOL_H
