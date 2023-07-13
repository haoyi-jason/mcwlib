#include "rcuprotocol.h"
#include <QDataStream>
#include <QMetaEnum>
#include <QDebug>

RcuProtocol::RcuProtocol(ProtocolDriver *pDriver)
{
    _driver = nullptr;

    _azbw = BANDWIDTH::BW_0HZ;
    _elbw = BANDWIDTH::BW_0HZ;
    _self_test = CONTROL_STATE::STATE_OFF;
    _record = CONTROL_STATE::STATE_OFF;
    _stow_az = CONTROL_STATE::STATE_OFF;
    _stow_el = CONTROL_STATE::STATE_OFF;
    _stow_control = CONTROL_STATE::STATE_OFF;
    _ant_stow = CONTROL_STATE::STATE_OFF;
    _predict_az_raw = 0x0;
    _predict_el_raw = 0x0;
    _flt_alpha_raw = 0x0;
    _flt_dt_raw = 0x0;
    _validChecksum = true;
    _reportInterval = 50;
    _stateReportInterval = 1000;
    _reportTimer = new QTimer();
    _stateReportTimer = new QTimer();
    _joystickMode = false;
    _rcuHostMode = false;

    _a2r_report = new A2R_Report();
    _a2r_state = new A2R_State();
    _sendReadySignal = true;

    connect(_reportTimer,&QTimer::timeout,this,&RcuProtocol::handleTimeout);
    connect(_stateReportTimer,&QTimer::timeout,this,&RcuProtocol::handleTimeout);
    setProtocolDriver(pDriver);
}

RcuProtocol::~RcuProtocol()
{
    if(_driver != nullptr){
        _driver->deleteLater();
    }
    _reportTimer->stop();
    _stateReportTimer->stop();

    delete _reportTimer;
    delete _stateReportTimer;
}

QString RcuProtocol::busName() const
{
    return _busName;
}

void RcuProtocol::setBusName(const QString &busName)
{
    _busName = busName;
}

QString RcuProtocol::connectionString() const
{
    return _connectionString;
}

void RcuProtocol::setConnectionString(const QString &connection)
{
    _connectionString = connection;
}

void RcuProtocol::start()
{
    if(_driver != nullptr){
        if(_driver->state() == ProtocolDriver::DISCONNECTED){
            if(_rcuHostMode){
                _driver->connectDevice();
//                _reportTimer->start(_reportInterval);
                _reportTimer->start(50);
            }
            else{
                _driver->connectDevice();
                _reportTimer->start(_reportInterval);
                _stateReportTimer->start(_stateReportInterval);
            }
        }
        connect(_driver,&ProtocolDriver::frameReceived,this,&RcuProtocol::frameRec,Qt::UniqueConnection);
        connect(_driver,&ProtocolDriver::stateChanged,this,&RcuProtocol::updateState);
        resetCounter();
    }
}

void RcuProtocol::stop()
{
    if(_driver != nullptr){
        _driver->disconnectDevice();
        _reportTimer->stop();
        _stateReportTimer->stop();
    }
}

void RcuProtocol::joystickMode(bool state)
{
    _joystickMode = state;
}

void RcuProtocol::hostMode(bool state)
{
    _rcuHostMode = state;
}

void RcuProtocol::setSimulatorData(QByteArray &data)
{
    _r2aData = data;
}

void RcuProtocol::valid_checksum(bool on)
{
    _validChecksum = on;
}

void RcuProtocol::set_report_interval(int ms)
{
    if(ms > 10){
        _reportInterval = ms;
    }
}

void RcuProtocol::set_state_report_interval(int ms)
{
    if(ms > 0){
        _stateReportInterval = ms;
    }
}

ProtocolDriver *RcuProtocol::driver() const
{
    return _driver;
}

void RcuProtocol::setProtocolDriver(ProtocolDriver *driver)
{
    if(_driver != nullptr){
        _driver->deleteLater();
    }
    _driver = driver;
//    if(_driver != nullptr){
//        if(_driver->state() == ProtocolDriver::DISCONNECTED){
//            _driver->connectDevice();
//            _reportTimer->start(_reportInterval);
//            _stateReportTimer->start(_stateReportInterval);
//        }
//        connect(_driver,&ProtocolDriver::frameReceived,this,&RcuProtocol::frameRec,Qt::UniqueConnection);
//        connect(_driver,&ProtocolDriver::stateChanged,this,&RcuProtocol::updateState);
//    }
//    else{
//        _reportTimer->stop();
//        _stateReportTimer->stop();
//    }
}

bool RcuProtocol::isConnected() const
{
    if(_driver == nullptr){
        return false;
    }
    return (_driver->state() == ProtocolDriver::CONNECTED);
}

bool RcuProtocol::canWrite() const
{
    return !((_driver == nullptr));
}

bool RcuProtocol::writeFrame(const QByteArray &data)
{
    if(!canWrite()){
        return false;
    }

    _driver->writeFrame(data);
    return true;
}

void RcuProtocol::frameRec()
{
    if(_driver == nullptr){
        return;
    }

    QByteArray data = _driver->readFrame();
    // parse frame here
    feedData(data);
    resetCounter();
}

void RcuProtocol::notifyForNewFrames()
{

}

void RcuProtocol::updateState()
{
    emit connectedChanged(isConnected());
}

void RcuProtocol::feedData(QByteArray &data)
{
    //qDebug()<<Q_FUNC_INFO;
    resetCounter();
    _receivedData.append(data);
    while(_receivedData.count()>0){
        if(_receivedData.at(0) != 0x5A)
            _receivedData.remove(0,1);
        else {
            break;
        }
    }
            if(_receivedData.size() >= 20){
                if(_receivedData.at(1) == 0x27){
                    bool vc = _r2a_report.unpackData(_receivedData);
                    bool valid = true;
                    if(_validChecksum){
                        valid &= vc;
                    }
                    valid &= (_r2a_report.cmd_type == 0x27);
                    valid &= (_r2a_report.length == 0x10);
                    _lastReportId = 0x27;

                    if(valid)
                    {
                        _a2r_state->INS = 1;
                        if(_r2a_report.AZ_BW != _azbw){
                            _azbw = (BANDWIDTH)_r2a_report.AZ_BW;
                            emit bandwidth_changed(RCU_AZ,_azbw);
                        }
                        if(_r2a_report.EL_BW != _elbw){
                            _elbw = (BANDWIDTH)_r2a_report.EL_BW;
                            emit bandwidth_changed(RCU_EL,_elbw);
                        }

                        if((_r2a_report.CONTROL_CMD & 0x01) == 0x01){
                            if(_self_test == STATE_OFF){
                                _self_test = STATE_ON;
                                emit state_changed(SELF_TEST,_self_test);
                            }
                        }
                        else{
                            if(_self_test == STATE_ON){
                                _self_test = STATE_OFF;
                                emit state_changed(SELF_TEST,_self_test);
                            }
                        }

                        if((_r2a_report.CONTROL_CMD & 0x02) == 0x02){
                            if(_record == STATE_OFF){
                                _record = STATE_ON;
                                emit state_changed(RECORD,_record);
                            }
                        }
                        else{
                            if(_record == STATE_ON){
                                _record = STATE_OFF;
                                emit state_changed(RECORD,_record);
                            }
                        }

                        if(_predict_az_raw != _r2a_report.Predict_AZ){
                            _predict_az_raw = _r2a_report.Predict_AZ;
                            _predict_az = (double)_predict_az_raw*360./65536.;
                            emit predict_changed(RCU_AZ,_predict_az_raw);
                        }
                        if(_predict_el_raw != _r2a_report.Predict_EL){
                            _predict_el_raw = _r2a_report.Predict_EL;
                            _predict_el = (double)_predict_el_raw*360./65536.;
                            emit predict_changed(RCU_EL,_predict_el_raw);
                        }

    //                    quint8 u8;
    //                    ds >> u8;
                        if(_r2a_report.Flt_Alpha != _flt_alpha_raw){
                            _flt_alpha_raw = _r2a_report.Flt_Alpha;
                            _flt_alpha = (double)_flt_alpha_raw/100.;
                            emit filter_changed(FILTER_ALPHA,_flt_alpha);
                        }
    //                    ds >> u8;
                        if(_r2a_report.Flt_Dt != _flt_dt_raw){
                            _flt_dt_raw = _r2a_report.Flt_Dt;
                            _flt_dt = (double)_flt_dt_raw/100.;
                            emit filter_changed(FILTER_DT,_flt_dt);
                        }

    //                    ds >> u8;
                        if((_r2a_report.Stow_AZ == 0x00) && (_stow_az == STATE_ON)){
                            _stow_az = STATE_OFF;
                            emit state_changed(AZ_STOW,_stow_az);
                        }
                        if((_r2a_report.Stow_AZ == 0x01) && (_stow_az == STATE_OFF)){
                            _stow_az = STATE_ON;
                            emit state_changed(AZ_STOW,_stow_az);
                        }

    //                    ds >> u8;
                        if((_r2a_report.Stow_EL == 0x00) && (_stow_el == STATE_ON)){
                            _stow_el = STATE_OFF;
                            emit state_changed(EL_STOW,_stow_el);
                        }
                        if((_r2a_report.Stow_EL == 0x01) && (_stow_el == STATE_OFF)){
                            _stow_el = STATE_ON;
                            emit state_changed(EL_STOW,_stow_el);
                        }

    //                    ds >> u8;
                        if((_r2a_report.Stow_Control == 0x00) && (_stow_control == STATE_ON)){
                            _stow_control = STATE_OFF;
                            emit state_changed(STOW_CONTROL,_stow_control);
                        }
                        if((_r2a_report.Stow_Control == 0x01) && (_stow_control == STATE_OFF)){
                            _stow_control = STATE_ON;
                            emit state_changed(STOW_CONTROL,_stow_control);
                        }

    //                    ds >> u8;
                        if((_r2a_report.ANT_STOW == 0x00) && (_ant_stow == STATE_ON)){
                            _ant_stow = STATE_OFF;
                            emit state_changed(ANT_STOW,_ant_stow);
                        }
                        if((_r2a_report.ANT_STOW == 0x01) && (_ant_stow == STATE_OFF)){
                            _ant_stow = STATE_ON;
                            emit state_changed(ANT_STOW,_ant_stow);
                        }
                        emit frameAvailable();
                        if(!_sendReadySignal){
                            _sendReadySignal = false;
                            emit rcuReady(true);
                        }
                    }
                }
                else if(_receivedData.at(1) == 0x40){
                    //qDebug()<<"receive joystick report";
                    _lastReportId = 0x40;
                    bool vc = _r2a_report_joystick.unpackData(_receivedData);
                    if(_predict_az_rpm_raw != _r2a_report_joystick.Velocity_AZ){
                        _predict_az_rpm_raw = _r2a_report_joystick.Velocity_AZ;
                        //double v = 360 * _predict_az_rpm_raw/65536;
                        emit predict_changed(RCU_AZS,_predict_az_rpm_raw);
                    }
                    if(_predict_el_rpm_raw != _r2a_report_joystick.Velocity_EL){
                        _predict_el_rpm_raw = _r2a_report_joystick.Velocity_EL;
                        //double v = 360 * _predict_az_rpm_raw/65536;
                        emit predict_changed(RCU_ELS,_predict_el_rpm_raw);
                    }
                    if(!_sendReadySignal){
                        _sendReadySignal = false;
                        emit rcuReady(true);
                    }
//                    emit frameAvailable();
                }
                else if(_receivedData.at(1) == 0x29){
                    bool vc = _a2r_state->unpackData(_receivedData);
                    emit frameAvailable();
                }
                else if(_receivedData.at(1) == 0x30){
                    bool vc = _a2r_report->unpackData(_receivedData);
                    emit frameAvailable();
                }
            _receivedData.clear();
    }
}

QStringList RcuProtocol::supportedBandwidth()
{
    QMetaEnum e = QMetaEnum::fromType<BANDWIDTH>();
    QStringList lst;
    for(int i=0;i<e.keyCount();i++){
        lst.append(e.key(i));
    }
    return lst;
}

QString RcuProtocol::summary() const
{
    QString msg = "";

    if(_lastReportId == 0x27){
        QMetaEnum e = QMetaEnum::fromType<BANDWIDTH>();
        QMetaEnum state = QMetaEnum::fromType<CONTROL_STATE>();

        msg += QString("AZ BW: %1, ").arg(e.key((int)_azbw));
        msg += QString("EL BW: %1\n").arg(e.key((int)_elbw));
        msg += QString("TEST MODE: %1 ").arg(state.key(_self_test));
        msg += QString("Record: %1 \n").arg(state.key(_record));
        msg += QString("Predict AZ: %1 [%2] ").arg(_predict_az_raw).arg(_predict_az);
        msg += QString("EL: %1 [%2]\n").arg(_predict_el_raw).arg(_predict_el);
        msg += QString("Filter Alpha: %1 ").arg((_flt_alpha_raw));
        msg += QString("DT:%1\n").arg((_flt_dt_raw));
        msg += QString("STOW AZ: %1 ").arg(state.key(_stow_az));
        msg += QString("EL: %1 ").arg(state.key(_stow_el));
        msg += QString("CONTROL: %1 ").arg(state.key(_stow_control));
        msg += QString("ANT: %1\n").arg(state.key(_ant_stow));
    }
    else if(_lastReportId == 0x40){
        msg += QString("Velocity AZ: %1 ").arg(_r2a_report_joystick.Velocity_AZ);
        msg += QString("EL: %1 \n").arg(_r2a_report_joystick.Velocity_EL);
    }

    return msg;
}

void RcuProtocol::handleTimeout()
{
    //qDebug()<<Q_FUNC_INFO;
    if(_driver == nullptr) return;

    _rcvCounter--;

    if(_rcvCounter == 0){
        emit rcuReady(false);
        _a2r_state->INS = 0;
        _sendReadySignal = true;
    }

    QTimer *tmr = (QTimer*)sender();
    if(tmr == _reportTimer){
        if(_rcuHostMode){
            if(_joystickMode){
                _driver->writeFrame(_r2a_report_joystick.packetData());
            }
            else{
                _driver->writeFrame(_r2a_report.packetData());
            }
        }
        else{
            _driver->writeFrame(_a2r_report->packetData());
        }
    }

    else if(tmr == _stateReportTimer){
        _driver->writeFrame(_a2r_state->packetData());
    }

}

int RcuProtocol::azbw() const
{
   return (int)_azbw;
}

void RcuProtocol::set_az_bw(int bw)
{
    _azbw = (BANDWIDTH)bw;
    _r2a_report.AZ_BW = bw;
}

int RcuProtocol::elbw() const
{
    return (int)_elbw;

}

void RcuProtocol::set_el_bw(int bw)
{
    _elbw = (BANDWIDTH)bw;
    _r2a_report.EL_BW = bw;
}

void RcuProtocol::set_selftest(bool set)
{
    _self_test = set? STATE_ON:STATE_OFF;
    if(set){
        _r2a_report.CONTROL_CMD |= 0x01;
    }
    else{
        _r2a_report.CONTROL_CMD &= ~0x01;
    }
}

void RcuProtocol::set_record(bool set)
{
    _record = set?STATE_ON:STATE_OFF;
    if(set){
        _r2a_report.CONTROL_CMD |= 0x02;
    }
    else{
        _r2a_report.CONTROL_CMD &= ~0x02;
    }
}

void RcuProtocol::set_stow_az(bool set)
{
    _stow_az = set?STATE_ON:STATE_OFF;

    _r2a_report.Stow_AZ = _stow_az;

    _a2r_state->AZ_STOW = (quint8)_stow_az;
}

void RcuProtocol::set_stow_el(bool set)
{
    _stow_el = set?STATE_ON:STATE_OFF;
    _r2a_report.Stow_EL = _stow_el;

    _a2r_state->EL_STOW = (quint8)_stow_el;
}
void RcuProtocol::set_stow_ctrl(bool set)
{
    _stow_control = set?STATE_ON:STATE_OFF;
    _r2a_report.Stow_Control = _stow_control;



}
void RcuProtocol::set_ant_stow(bool set)
{
    _ant_stow = set?STATE_ON:STATE_OFF;
    _r2a_report.ANT_STOW = _ant_stow;

    _a2r_state->ANT_STOW = _ant_stow;
}

void RcuProtocol::set_predict_az(quint16 value)
{
    _predict_az_raw = value;
    _r2a_report.Predict_AZ = value;
    _a2r_report->AP_AZ = value;

}
void RcuProtocol::set_predict_el(quint16 value)
{
    _predict_el_raw = value;
    _r2a_report.Predict_EL = value;
    _a2r_report->AP_EL = value;
}

void RcuProtocol::set_predict_az_rpm(quint16 value)
{
    _predict_az_rpm_raw = value;
    _r2a_report_joystick.Velocity_AZ = value;
}
void RcuProtocol::set_predict_el_rpm(quint16 value)
{
    _predict_el_rpm_raw = value;
    _r2a_report_joystick.Velocity_EL = value;
}

void RcuProtocol::set_az_m1(bool set)
{
    _a2r_state->APDU_AZ1=set?0x01:0x00;
}

void RcuProtocol::set_az_m2(bool set)
{
    _a2r_state->APDU_AZ2 = set?0x01:0x00;
}

void RcuProtocol::set_el_m1(bool set)
{
    _a2r_state->APDU_EL1 = set?0x01:0x00;
}

void RcuProtocol::set_el_m2(bool set)
{
    _a2r_state->APDU_EL2 = set?0x01:0x00;
}

bool RcuProtocol::selfTest() const
{
    return (_self_test == STATE_ON);
}
bool RcuProtocol::record() const
{
    return (_record == STATE_ON);
}
bool RcuProtocol::stow_az() const
{
    return (_stow_az == STATE_ON);
}
bool RcuProtocol::stow_el() const
{
    return (_stow_el == STATE_ON);
}
bool RcuProtocol::stow_ctrl() const
{
    return (_stow_control == STATE_ON);
}
bool RcuProtocol::ant_stow() const
{
    return (_ant_stow == STATE_ON);
}

quint16 RcuProtocol::predictAZ() const
{
    return _predict_az_raw;
}
quint16 RcuProtocol::predictEL() const
{
    return _predict_el_raw;
}
quint16 RcuProtocol::filterAlpha() const
{
    return _flt_alpha_raw;
}
quint16 RcuProtocol::filterDt() const
{
    return _flt_dt_raw;
}

A2R_Report *RcuProtocol::a2rReport() const
{
    return _a2r_report;
}

A2R_State *RcuProtocol::a2rState() const
{
    return _a2r_state;
}

void RcuProtocol::resetCounter()
{
    _rcvCounter = 5000/_reportInterval;
}
