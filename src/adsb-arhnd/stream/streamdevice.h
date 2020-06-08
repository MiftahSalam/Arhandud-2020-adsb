#ifndef STREAMDEVICE_H
#define STREAMDEVICE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#include "adsb-arhnd_global.h"

namespace AdsbArhnd {

enum StreamMode
{
    In = 0,
    Out,
    Idle
};

enum StreamType
{
    Serial = 0,
    TCP_CLIENT,
    UDP,
    TCP_SERVER
};

struct StreamSettings
{
    StreamMode mode;
    StreamType type;
    QString config;
};

enum SensorStatus
{
    NOT_AVAIL = 0,
    NO_DATA,
    DATA_UNKNOWN,
    AVAIL
};

class ADSB_ARHND_EXPORT StreamDevice : public QObject
{
    Q_OBJECT
public:
    explicit StreamDevice(QObject *parent = 0, StreamSettings settings = StreamSettings());

    QByteArray readData();
    void sendData(QByteArray data);
    QString getCurrentError() { return m_error; }
    bool isTcpConnected()
    {
        m_error = tcpsocket.state()==QAbstractSocket::ConnectedState ? "" : m_error;
        return tcpsocket.state()==QAbstractSocket::ConnectedState;
    }

signals:

private slots:

private:
    QTcpSocket tcpsocket;
    QUdpSocket udpsocket;
    StreamSettings m_settings;
    QString m_error;

    void init();
    QString setTcpClient();
    QString setUdp();
};
}

#endif // STREAMDEVICE_H
