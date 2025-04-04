#include "streamdevice.h"

#include <QStringList>
#include <QDebug>
#include <QHostInfo>
#include <QEventLoop>
#include <QTimer>

using namespace AdsbArhnd;

HttpHandler::HttpHandler() : last_request(QDateTime::currentDateTime())
{

}

QByteArray HttpHandler::getAircraftData()
{
    QUrl url_aircraft = QUrl(main_url.toString()+"/aircraftlist.json");
    return getData(url_aircraft);
}

QByteArray HttpHandler::getADSBInfo()
{
    QUrl url_info = QUrl(main_url.toString()+"/mapinfo.json");
    return getData(url_info);
}

QByteArray HttpHandler::getData(QUrl url)
{
    QDateTime now = QDateTime::currentDateTime();

    if(last_request.secsTo(now) < 2)
        return "";

    qDebug()<<Q_FUNC_INFO<<"url"<<url;
    last_request = now;
    req.setUrl(url);
    req.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.125 Safari/537.36");

    reply = qnam.get(req);

    QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    QTimer::singleShot(3000,&loop,SLOT(quit()));
    loop.exec();
    m_error = reply->errorString().contains("Unknown") ? "" : reply->errorString();
    qDebug()<<Q_FUNC_INFO<<m_error;

    return reply->readAll();

}

QString HttpHandler::setup(QUrl url)
{
    main_url = url;
    return "";
}

StreamDevice::StreamDevice(QObject *parent, StreamSettings settings) :
    QObject(parent),m_settings(settings)
{
    //    serialPort = NULL;

    init();
}

void StreamDevice::sendData(QByteArray data)
{
    if(m_settings.type == TCP_CLIENT)
    {
        if(tcpsocket.state() == QAbstractSocket::ConnectedState)
        {
            tcpsocket.write(data);
            tcpsocket.waitForBytesWritten(50);
        }
        else
            m_error = setTcpClient();
    }
}

QByteArray StreamDevice::readData()
{
    QByteArray ret_val = "tes";
    ret_val.clear();

    if(m_settings.type == UDP)
    {
        if(udpsocket.state() == QAbstractSocket::BoundState)
        {
            udpsocket.waitForReadyRead(10);
            while(udpsocket.hasPendingDatagrams())
            {
//                qDebug()<<Q_FUNC_INFO<<udpsocket.bytesAvailable();
                ret_val.resize(udpsocket.pendingDatagramSize());
                udpsocket.readDatagram(ret_val.data(),ret_val.size());
            }
        }
    }
    else if(m_settings.type == HTTP)
    {
        ret_val = http.getAircraftData();
        m_error = http.getError();
        if(!m_error.isEmpty())
        {
            ret_val = http.getADSBInfo();
            m_error = http.getError();
        }

//        qDebug()<<Q_FUNC_INFO<<ret_val<<m_error;
    }
    /*
    else if(m_settings.type == Serial && (serialPort != NULL) )
    {
        serialPort->waitForReadyRead(50);

        if(serialPort->bytesAvailable() > 0)
            ret_val = serialPort->readAll();
    }
    */
    return ret_val;
}

QString StreamDevice::setUdp()
{
    QStringList config_list = m_settings.config.split(";");

    if(config_list.size() != 2)
    {
        qWarning()<<Q_FUNC_INFO<<"UDP invalid config. config not complete";
        return "Invalid config";
    }

    QHostInfo info = QHostInfo::fromName(config_list.at(0));
    if(info.error() == QHostInfo::HostNotFound)
        return "Host not found";

    if(config_list.at(1).toUInt() > 65535)
        return "UDP port out of range";

    qDebug()<<Q_FUNC_INFO<<"host info"<<info.errorString();

    udpsocket.close();
    if(!udpsocket.bind(QHostAddress(config_list.at(0)),config_list.at(1).toUInt()))
        qWarning()<<Q_FUNC_INFO<<udpsocket.errorString()<<udpsocket.state();

    return udpsocket.errorString().contains("Unknown") ? "" : udpsocket.errorString();
}


QString StreamDevice::setHttp()
{
    QStringList config_list = m_settings.config.split(";");

    if(config_list.size() != 2)
    {
        qDebug()<<Q_FUNC_INFO<<"invalid config. config not complete";
        return "Invalid config";
    }

    QUrl url_info = QUrl("http://"+config_list.at(0)+":"+config_list.at(1));

    QString error = http.setup(url_info);
    qDebug()<<Q_FUNC_INFO<<error;

    return error.contains("Unknown") ? "" : error;
}

QString StreamDevice::setTcpClient()
{
    QStringList config_list = m_settings.config.split(";");

    if(config_list.size() != 2)
    {
        qDebug()<<Q_FUNC_INFO<<"invalid config. config not complete";
        return "Invalid config";
    }

    tcpsocket.close();
    tcpsocket.connectToHost(QHostAddress(config_list.at(0)),config_list.at(1).toUInt());
    tcpsocket.waitForConnected(100);

//    qDebug()<<Q_FUNC_INFO<<tcpsocket.errorString();
    return tcpsocket.errorString().contains("Unknown") ? "" : tcpsocket.errorString();
}

/*
QString StreamDevice::setSerial()
{
    QStringList config_list = m_settings.config.split(";");

    if(config_list.size() != 6)
    {
        qDebug()<<Q_FUNC_INFO<<"invalid config. config not complete";
        return "Invalid config";
    }

    if(serialPort == NULL)
        serialPort = new QextSerialPort(QextSerialPort::Polling, this);

    if(serialPort->isOpen())
        serialPort->close();

    serialPort->setPortName(config_list.at(0));

    if(m_settings.mode == In)
        serialPort->open(QIODevice::ReadOnly);
    else if(m_settings.mode == Out)
        serialPort->open(QIODevice::WriteOnly);

    if(serialPort->isOpen())
    {
        if(setBaud(config_list.at(1))<2400)
        {
            qDebug()<<Q_FUNC_INFO<<"invalid baudrate parameter '"<<config_list.at(1)<<"'";
            return "Invalid baudrate";
        }
        if(setDataBits(config_list.at(2))<0)
        {
            qDebug()<<Q_FUNC_INFO<<"invalid databits parameter '"<<config_list.at(2)<<"'";
            return "Invalid databits";
        }
        if(setStopBit(config_list.at(3))<0)
        {
            qDebug()<<Q_FUNC_INFO<<"invalid stopbit parameter '"<<config_list.at(3)<<"'";
            return "Invalid stopbit";
        }
        if(setParity(config_list.at(4))<0)
        {
            qDebug()<<Q_FUNC_INFO<<"invalid parity parameter '"<<config_list.at(4)<<"'";
            return "Invalid parity";
        }
        if(setFlowControl(config_list.at(5))<0)
        {
            qDebug()<<Q_FUNC_INFO<<"invalid Flow Control parameter '"<<config_list.at(5)<<"'";
            return "Invalid Flow Control";
        }

        serialPort->setBaudRate(setBaud(config_list.at(1)));
        serialPort->setDataBits(setDataBits(config_list.at(2)));
        serialPort->setStopBits(setStopBit(config_list.at(3)));
        serialPort->setParity(setParity(config_list.at(4)));
        serialPort->setFlowControl(setFlowControl(config_list.at(5)));
        serialPort->setQueryMode(QextSerialPort::Polling);

        qDebug()<<Q_FUNC_INFO<<"Serial port ready";
        return "";

    }
    else
    {
        qDebug()<<Q_FUNC_INFO<<"Cannot open device '"<<serialPort->portName()<<"' with error '"<<serialPort->errorString()<<"'";
        return "Cannot open device";
    }

}
*/
void StreamDevice::init()
{
    qDebug()<<Q_FUNC_INFO<<m_settings.config<<m_settings.mode<<m_settings.type;

    if(m_settings.type == TCP_CLIENT)
        m_error = setTcpClient();
    /*
    else if(m_settings.type == Serial)
        m_error = setSerial();
    */
    else if(m_settings.type == UDP)
        m_error = setUdp();
    else if(m_settings.type == HTTP)
        m_error = setHttp();
}
/*
BaudRateType StreamDevice::setBaud(QString stringBaud)
{
    if (!QString::compare(stringBaud,"300"))
        return BAUD300;
    else if (!QString::compare(stringBaud,"600"))
        return BAUD600;
    else if (!QString::compare(stringBaud,"1200"))
        return BAUD1200;
    else if (!QString::compare(stringBaud,"2400"))
        return BAUD2400;
    else if (!QString::compare(stringBaud,"4800"))
        return BAUD4800;
    else if (!QString::compare(stringBaud,"9600"))
        return BAUD9600;
    else if (!QString::compare(stringBaud,"19200"))
        return BAUD19200;
    else if (!QString::compare(stringBaud,"38400"))
        return BAUD38400;
    else if (!QString::compare(stringBaud,"57600"))
        return BAUD57600;
    else if (!QString::compare(stringBaud,"115200"))
        return BAUD115200;
    else
        return (BaudRateType)stringBaud.toInt();
}

DataBitsType StreamDevice::setDataBits(QString stringDataBits)
{
    if(!QString::compare(stringDataBits,"5"))
        return DATA_5;
    else if (!QString::compare(stringDataBits,"6"))
        return DATA_6;
    else if (!QString::compare(stringDataBits,"7"))
        return DATA_7;
    else if (!QString::compare(stringDataBits,"8"))
        return DATA_8;
    else
        return (DataBitsType) -1;

}

ParityType StreamDevice::setParity(QString stringParity)
{
    if(!QString::compare(stringParity,"NONE"))
        return PAR_NONE;
    else if (!QString::compare(stringParity,"ODD"))
        return PAR_ODD;
    else if (!QString::compare(stringParity,"EVEN"))
        return PAR_EVEN;
    else if (!QString::compare(stringParity,"SPACE"))
        return PAR_SPACE;
    else
        return (ParityType) -1;
}
StopBitsType StreamDevice::setStopBit(QString stringStopBit)
{
    if(!QString::compare(stringStopBit,"1"))
        return STOP_1;
    else if (!QString::compare(stringStopBit,"2"))
        return STOP_2;
    else
        return (StopBitsType) -1;
}
FlowType StreamDevice::setFlowControl(QString stringFlowControl)
{
    if(!QString::compare(stringFlowControl,"OFF"))
        return FLOW_OFF;
    else if (!QString::compare(stringFlowControl,"HARDWARE"))
        return FLOW_HARDWARE;
    else if (!QString::compare(stringFlowControl,"XONXOFF"))
        return FLOW_XONXOFF;
    else
        return (FlowType) -1;
}
*/
