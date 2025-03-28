#include "stream.h"

#include <log4qt/logger.h>

#include <QStringList>
#include <QDebug>
#include <QDataStream>
#include <QDateTime>

using namespace AdsbArhnd;

Stream::Stream(QThread *parent,StreamSettings settings) :
    QThread(parent), m_settings(settings),m_status(NOT_AVAIL)
{
    device = NULL;
    init();
}

void Stream::loop()
{
    qDebug()<<Q_FUNC_INFO;

    quint64 data_tick = QDateTime::currentSecsSinceEpoch();
    quint64 now = data_tick;
    quint64 update_tick = data_tick;

    while(true)
    {
        now = QDateTime::currentSecsSinceEpoch();

//        qDebug()<<Q_FUNC_INFO;
        if(m_settings.mode == In)
        {
            m_data = device->readData();

            if(!m_data.isEmpty())
            {
//                qDebug()<<Q_FUNC_INFO<<"ada data"<<m_data;
                data_tick = now;
                decode();
                emit signal_dataReceived(m_data); //untuk display raw data
            }

//            qDebug()<<Q_FUNC_INFO<<m_data<<now<<data_tick;
            if((now-data_tick) > 10)
            {
//                qDebug()<<Q_FUNC_INFO<<"no data";
                m_status = NO_DATA;
                update();
                data_tick = now;
                Log4Qt::Logger::rootLogger()->trace()<<Q_FUNC_INFO<<"sleep";
                sleep(5);
            }

            if((now-update_tick) > 1)
            {
                update();
                update_tick = now;
                Log4Qt::Logger::rootLogger()->trace()<<Q_FUNC_INFO<<"update";
            }
        }
        msleep(10);

        if(exitReq)
            break;
    }
}

void Stream::sendData()
{
//    qDebug()<<Q_FUNC_INFO;
    m_data.append(encode());
    emit signal_dataEncoded(m_data);
    m_data.clear();
}

void Stream::init()
{
    qDebug()<<Q_FUNC_INFO<<"first"<<m_settings.config<<m_settings.mode<<m_settings.type<<isRunning();

    while(isRunning())
    {
        mutex.tryLock(1000);
        exitReq = true;
        mutex.unlock();
    }
    qDebug()<<Q_FUNC_INFO<<"end"<<m_settings.config<<m_settings.mode<<m_settings.type<<isRunning();

    m_data.clear();
    exitReq = false;
    m_data_error_tick = 0;

    if(device)
        delete device;

    device = new StreamDevice(this,m_settings);
    m_error = device->getCurrentError();

    if(m_settings.mode == In && m_error.isEmpty())
        start();

}
