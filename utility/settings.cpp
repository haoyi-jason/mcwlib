#include "settings.h"
#include <QDebug>
#include <QCoreApplication>
#include <QTextCodec>
#include <QDateTime>
#include <QTextStream>
#include <QDir>

QScopedPointer<Settings> Settings::_instance;

Settings::Settings(QObject *parent) : QObject(parent)
{
 //   qDebug()<<Q_FUNC_INFO;
//    _axisConfig.append(new AxisConfig());
//    _axisConfig.append(new AxisConfig());

//    _dacConfig.append(new AD5764Config());
//    _dacConfig.append(new AD5764Config());
//    _dacConfig.append(new AD5764Config());
//    _dacConfig.append(new AD5764Config());


}

Settings::~Settings()
{
    qDebug()<<Q_FUNC_INFO;
}

void Settings::LoadConfig()
{
    // load settings from setting file
    QString path = QCoreApplication::applicationDirPath();
    path  += "//config.ini";
    QSettings *setting = new QSettings(path, QSettings::IniFormat);
    setting->setIniCodec(QTextCodec::codecForName("UTF-8"));
    int sz = 0;
//    int sz = setting->beginReadArray("AXIS_CONFIG");
//    if(sz > 0){
//        for(int i=0;i<sz;i++){
//            AxisConfig *cfg = new AxisConfig();
//            setting->setArrayIndex(i);
//            cfg->pos_limit_low = setting->value("POS_LIMIT_LOW").toDouble();
//            cfg->pos_limit_high = setting->value("POS_LIMIT_HIGH").toDouble();
//            cfg->speed_low = setting->value("RPM_LOW").toDouble();
//            cfg->speed_high = setting->value("RPM_HIGH").toDouble();
//            cfg->output_low = setting->value("OUTPUT_LOW").toDouble();
//            cfg->output_high = setting->value("OUTPUT_HIGH").toDouble();
//            cfg->skp = setting->value("SKP").toDouble();
//            cfg->ski = setting->value("SKI").toDouble();
//            cfg->skd = setting->value("SKD").toDouble();
//            cfg->pkp = setting->value("PKP").toDouble();
//            cfg->pki = setting->value("PKI").toDouble();
//            cfg->pkd = setting->value("PKD").toDouble();
//            instance()._axisConfig.append(cfg);
//        }
//        setting->endArray();
//    }

    setting->beginGroup("RCU");
    //if(sz==1){
        _rcuConfig = new RcuConfig();
        _rcuConfig->Interface = (setting->value("TYPE").toString().contains("UDP"))?RcuConfig::RCU_USE_UDP:RcuConfig::RCU_USE_422;
        _rcuConfig->ConnectString = setting->value("CONNECTION").toString();
        if(_rcuConfig->Interface == RcuConfig::RCU_USE_UDP){
            _rcuConfig->tx_port = setting->value("TX_PORT").toInt();
            _rcuConfig->rx_port = setting->value("RX_PORT").toInt();
        }
        else{
            _rcuConfig->baudrate = setting->value("BITRATE").toInt();
        }
        _rcuConfig->valid_checksum = setting->value("VALID_CRC").toString()=="TRUE"?true:false;
        _rcuConfig->reportInterval = setting->value("REPORT_INTERVAL").toInt();
        _rcuConfig->stateReportInterval = setting->value("STATE_REPORT").toInt();
    //}
    setting->endGroup();

    sz = setting->beginReadArray("SERIAL");
    if(sz > 0){
        for(int i=0;i<sz;i++){
            setting->setArrayIndex(i);
            SerialPortConfig *cfg = new SerialPortConfig();
            cfg->connection = setting->value("PORT").toString();
            cfg->baudrate = setting->value("BAUDRATE").toInt();
            cfg->parity = setting->value("PARITY").toString();
            cfg->databits = setting->value("DATABITS").toInt();
            cfg->stopbits = setting->value("STOPBITS").toString();
            cfg->flowControl = setting->value("FLOWCONTROL").toString();
            _serialPortConfig.append(cfg);
        }
    }
    setting->endArray();

    sz = setting->beginReadArray("CANBUS");
    if(sz > 0){
        for(int i=0;i<sz;i++){
            setting->setArrayIndex(i);
            SerialPortConfig *cfg = new SerialPortConfig();
            cfg->connection = setting->value("PORT").toString();
            cfg->baudrate = setting->value("BITRATE").toInt();
            cfg->mode = setting->value("MODE").toString();
            _canPortConfig.append(cfg);
        }
    }
    setting->endArray();

    sz = setting->beginReadArray("ANALOG_OUTPUTS");
    if(sz > 0){
        for(int i=0;i<sz;i++){
            AD5764Config *cfg = new AD5764Config();
            cfg->CorseGain =(AD5764Config::CORSE_GAIN)setting->value("CORSE_GAIN").toInt();
            cfg->FineGain = setting->value("FINE_GAIN").toInt();
            cfg->Offset = setting->value("OFFSET").toInt();
            _dacConfig.append(cfg);
        }
    }
    setting->endArray();

    setting->beginGroup("LOG");
    if(setting->value("PATH").isNull()){
        _logPath = QString("%1/Log").arg(QCoreApplication::applicationDirPath());
    }
    else{
        _logPath = QString("%1").arg(setting->value("PATH").toString());
    }
    setting->endGroup();
    if(!QDir(_logPath).exists()){
        QDir().mkpath(_logPath);
        instance().Info("Create Log Path");
    }

}

void Settings::StoreConfig()
{
    QString path = QCoreApplication::applicationDirPath();
    path  += "//config.ini";
    QSettings *setting = new QSettings(path, QSettings::IniFormat);
    setting->setIniCodec(QTextCodec::codecForName("UTF-8"));

    setting->beginWriteArray("AXIS_CONFIG");
    for(int i=0;i<instance()._axisConfig.count();i++){
        AxisConfig *cfg = instance()._axisConfig.at(i);
        setting->setArrayIndex(i);

        setting->setValue("POS_LIMIT_LOW",cfg->pos_limit_low);
        setting->setValue("POS_LIMIT_HIGH",cfg->pos_limit_high);
        setting->setValue("RPM_LOW",cfg->speed_low);
        setting->setValue("RPM_HIGH",cfg->speed_high);
        setting->setValue("OUTPUT_LOW",cfg->output_low);
        setting->setValue("OUTPUT_HIGH",cfg->output_high);
        setting->setValue("SKP",cfg->skp);
        setting->setValue("SKI",cfg->ski);
        setting->setValue("SKD",cfg->skd);
        setting->setValue("PKP",cfg->pkp);
        setting->setValue("PKI",cfg->pki);
        setting->setValue("PKD",cfg->pkd);
    }
    setting->endArray();
    setting->sync();

    setting->beginGroup("RCU");
    if(_rcuConfig != nullptr){
        setting->setValue("TYPE",_rcuConfig->Interface == RcuConfig::RCU_USE_UDP?"UDP":"RS422");
        setting->setValue("CONNECTION",_rcuConfig->ConnectString);
        setting->setValue("TX_PORT",_rcuConfig->tx_port );
        setting->setValue("RX_PORT",_rcuConfig->rx_port );
        setting->setValue("BITRATE",QString::number(_rcuConfig->baudrate));
    }
    setting->endGroup();
    setting->sync();

    setting->beginWriteArray("SYSTEM");

    setting->endArray();

    setting->beginWriteArray("SERIAL");
    for(int i=0;i<_serialPortConfig.count();i++){
        setting->setArrayIndex(i);
        SerialPortConfig *cfg = _serialPortConfig.at(i);
        setting->setValue("PORT"       ,cfg->connection );
        setting->setValue("BAUDRATE"   ,QString::number(cfg->baudrate   ));
        setting->setValue("PARITY"     ,cfg->parity     );
        setting->setValue("DATABITS"   ,cfg->databits   );
        setting->setValue("STOPBITS"   ,cfg->stopbits   );
        setting->setValue("FLOWCONTROL",cfg->flowControl);
    }
    setting->endArray();
    setting->sync();

    setting->beginWriteArray("CANBUS");
    for(int i=0;i<_canPortConfig.count();i++){
        setting->setArrayIndex(i);
        SerialPortConfig *cfg = _canPortConfig.at(i);
        setting->setValue("PORT"   ,cfg->connection);
        setting->setValue("BITRATE",QString::number(cfg->baudrate  ));
        setting->setValue("MODE"   ,cfg->mode      );
    }
    setting->endArray();
    setting->sync();

    setting->beginWriteArray("ANALOG_OUTPUTS");
    for(int i=0;i<_dacConfig.count();i++){
        setting->setArrayIndex(i);
        AD5764Config *cfg = _dacConfig.at(i);
        setting->setValue("CORSE_GAIN",cfg->CorseGain);
        setting->setValue("FINE_GAIN" ,cfg->FineGain );
        setting->setValue("OFFSET"    ,cfg->Offset   );
    }
    setting->endArray();
    setting->sync();

    setting->beginGroup("LOG");
    setting->setValue("PATH",_logPath);
    setting->endGroup();
    setting->sync();

}

Settings &Settings::instance()
{
    if(_instance.isNull()){
//        _mutex.lock();
        _instance.reset(new Settings());
//        _mutex.unlock();
    }
    return *_instance.data();
}

//void Settings::release()
//{
//    if(_instance != NULL){
//        _mutex.lock();
//        delete _instance;
//        _instance = NULL;
//        _mutex.unlock();
//    }
//}

QString Settings::getInstanceName() const
{
    return "Settings Signleton";
}

AxisConfig *Settings::config(RCU_AXIS axis)
{
    switch(axis){
    case AZ:
        return instance()._axisConfig.at(0);
        break;
    case EL:
        return instance()._axisConfig.at(1);
        break;
    }
    return nullptr;
}
AD5764Config *Settings::dacConfig(int channel)
{
    if(channel < instance()._dacConfig.count()){
        return instance()._dacConfig.at(channel);
    }
    return nullptr;
}
SerialPortConfig *Settings::serialConfig(int channel)
{
    if(channel < instance()._serialPortConfig.count()){
        return instance()._serialPortConfig.at(channel);
    }
    return nullptr;
}
void Settings::setSerialPortConfig(SerialPortConfig *cfg, int channel)
{
    if(channel < instance()._serialPortConfig.count()){
        instance()._serialPortConfig[channel] = cfg;
    }
}

SerialPortConfig *Settings::canConfig(int channel)
{
    if(channel < instance()._canPortConfig.count()){
        return instance()._canPortConfig.at(channel);
    }
    return nullptr;
}

void Settings::setCanConfig(SerialPortConfig *cfg, int channel)
{
    if(channel < instance()._canPortConfig.count()){
        //instance()._canPortConfig.replace(channel,cfg);
        instance()._canPortConfig[channel] = cfg;
    }
}

RcuConfig *Settings::rcuConfig()
{
    return instance()._rcuConfig;
}


int Settings::SerialPortCount()
{
    return instance()._serialPortConfig.count();
}


int Settings::CanPortCount()
{
    return instance()._canPortConfig.count();
}
QStringList Settings::supportedSerialPorts()
{
    QStringList nameList;
    foreach (SerialPortConfig *v, instance()._serialPortConfig) {
        nameList.append(v->connection);
    }
    return nameList;
}

void Settings::startCanBus()
{

}

QString Settings::logPath()
{
    return _logPath;
}


void Settings::Info(const QString &msg)
{
    QString dateString = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString formattedTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz");

//    QString path = QString("%1").arg(instance()._logPath);
//    if(!QDir(path).exists()){
//        QDir().mkpath(path);
//    }

    //qDebug()<<"Log Path:" << instance()._logPath;
    if(!QDir().exists(instance()._logPath)){
        QDir().mkpath(instance()._logPath);
    }
    QString filePath = QString("%1/%2-log.txt").arg(instance()._logPath,dateString);
    QString txt = QString("[INFO] %1 %2").arg(formattedTime, msg);
    QFile outFile(filePath);
    //qDebug()<<"Log File:"<<filePath;
    if(outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
    }

    //qDebug()<<Q_FUNC_INFO<<"Log File:"<<filePath;

}

void Settings::Warning(const QString &msg)
{
    QString dateString = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString formattedTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz");

//    QString path = QString("%1/%2").arg(QDir::currentPath(), instance()._logPath);
//    if(!QDir(path).exists()){
//        QDir().mkpath(path);
//    }

    if(!QDir().exists(instance()._logPath)){
        QDir().mkpath(instance()._logPath);
    }

    QString filePath = QString("%1\\%2-log.txt").arg(instance()._logPath,dateString);
    QString txt = QString("[WARNING] %1 %2").arg(formattedTime, msg);
    QFile outFile(filePath);
    if(outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
    }

}

void Settings::Debug(const QString &msg)
{
    QString dateString = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString formattedTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz");

//    QString path = QString("%1").arg(instance()._logPath);
//    if(!QDir(path).exists()){
//        QDir().mkpath(path);
//    }

    if(!QDir().exists(instance()._logPath)){
        QDir().mkpath(instance()._logPath);
    }
    QString filePath = QString("%1\\%2-log.txt").arg(instance()._logPath,dateString);
    QString txt = QString("[DEBUG] %1 %2").arg(formattedTime, msg);
    QFile outFile(filePath);

    if(outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
    }

}
