#ifndef ADSBSTREAM_H
#define ADSBSTREAM_H

#include <QObject>

#include <stream-arhnd/stream/stream.h>
#include "adsb.h"

namespace AdsbArhnd {

using namespace StreamArhnd;

class ADSB_ARHND_EXPORT ADSBStreamIn : public Stream
{
    Q_OBJECT
public:
    explicit ADSBStreamIn(Stream *parent = 0, StreamSettings settings = StreamSettings());

    ADSBDecoder getADSB();
    void setLatLon(double lat,double lon);
    void setTargetFromIFF(ADSBTargetData track);
    void setTargetIdentityFromIFF(const QString squawk, const quint8 identity);

signals:
    void signal_newTarget(int icao);
    void signal_removeTarget(int icao);

private slots:

protected:
    void decode();
    void update();
    QByteArray encode() {}
    void run();

private:
    ADSBParser adsbParser;
    ADSBDecoder adsbDecoder;
};

class ADSB_ARHND_EXPORT ADSBStream : public QObject
{
    Q_OBJECT
public:
    explicit ADSBStream(QObject *parent = 0,
                        StreamSettings inSettings = StreamSettings());

    ADSBDecoder getADSB();
    QString getCurrentInputError() { return this->m_InError; }
    SensorStatus getCurrentSensorStatus() { return this->inStream->getSensorStatus(); }
    void setInSettings(StreamSettings inSettings);
    void setLatLon(double lat,double lon) { inStream->setLatLon(lat,lon); }
    void setTargetFromIFF(ADSBTargetData track) { inStream->setTargetFromIFF(track); }
    void setTargetIdentityFromIFF(const QString squawk, const quint8 identity)
    { inStream->setTargetIdentityFromIFF(squawk,identity); }

    ~ADSBStream();

signals:
    void signal_sendStreamData(QByteArray data);
    void signal_updateTargetData(QByteArray data);

private slots:
    void trigger_updateTargetData(int icao);

private:
    ADSBStreamIn *inStream;
    QString m_InError;
};
}

#endif // ADSBSTREAM_H
