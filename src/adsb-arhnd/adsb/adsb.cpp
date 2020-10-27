#include "adsb.h"

#include <QDebug>
#include <QDateTime>
#include <math.h>

using namespace AdsbArhnd;

ADSBDecoder::ADSBDecoder()
{
    cur_targets_icao.clear();
    targetListMap.clear();
}

void ADSBDecoder::setTargetNumber(int icao, int number)
{
    ADSBTargetData* cur_target = targetListMap.value(icao);
    cur_target->number = number;
    targetListMap.insert(icao,cur_target);
}

void ADSBDecoder::updateADSB()
{
//    qDebug()<<Q_FUNC_INFO<<"targetListMap->size"<<targetListMap.size();

    QHashIterator<int,ADSBTargetData*> i(targetListMap);

    while(i.hasNext())
    {
        i.next();

        ADSBTargetData *buf_data = i.value();

        if(IsExpired(buf_data->icao))
        {
            deleteTarget(buf_data->icao);
            continue;
        }
    }
}

bool ADSBDecoder::IsExpired(int icao)
{
    QDateTime time = QDateTime::currentDateTime();

    return (time.toTime_t() - targetListMap.value(icao)->time_stamp > ADSB_TARGET_EXPIRED);
}

void ADSBDecoder::deleteTarget(int icao)
{
    targetListMap.remove(icao);
//    qDebug()<<Q_FUNC_INFO<<icao;
}

QList<int> ADSBDecoder::decode(QJsonArray targets)
{
    cur_targets_icao.clear();

//    qDebug()<<Q_FUNC_INFO<<targets.size();

    for(int i=0; i<targets.size(); i++)
    {
        QJsonObject target = targets.at(i).toObject();

        bool ok;
        int icao = target.value("hex").toString("-1").toInt(&ok,16);

        if(ok && (icao > 0))
        {
            ADSBTargetData *cur_target;

            if(targetListMap.contains(icao))
                cur_target = targetListMap.take(icao);
            else
                cur_target = new ADSBTargetData();

            QString call_sign = target.value("fli").toString("");
            float lat = target.value("lat").toDouble(1000);
            float lon = target.value("lon").toDouble(1000);
            float alt = target.value("alt").toDouble(1000000);
            float speed = target.value("spd").toDouble(10000);
            float course = target.value("trk").toDouble(1000);
            float vertical_rate = target.value("vrt").toDouble(10000);
            quint8 ground = target.value("gda").toString("G").contains("G") ? 0 : 1;
            QString country = target.value("cou").toString("");

            cur_target->icao = icao;
            cur_target->trimmed_call_sign = call_sign;
            strncpy(cur_target->call_sign,call_sign.toUtf8().constData(),call_sign.size());
            cur_target->lat = lat;
            cur_target->lon = lon;
            cur_target->alt = alt+150.; //jangan terlalu mirip
            cur_target->speed = speed+2.3; //jangan terlalu mirip
            cur_target->course = course+1.3; //jangan terlalu mirip
            cur_target->vertical_rate = vertical_rate;
            cur_target->ground = ground;
            cur_target->trimmed_country = country;
            strncpy(cur_target->country,country.toUtf8().constData(),country.size());

            if(fabs(lat) <= 90)
                cur_target->lat_valid = true;
            else
                cur_target->lat_valid = false;

            if(fabs(lon) <= 180)
                cur_target->lon_valid = true;
            else
                cur_target->lon_valid = false;

            if(fabs(alt) < 1000000)
                cur_target->alt_valid = true;
            else
                cur_target->alt_valid = false;

            if(fabs(speed) < 10000)
                cur_target->speed_valid = true;
            else
                cur_target->speed_valid = false;

            if(fabs(course) <= 360)
                cur_target->course_valid = true;
            else
                cur_target->course_valid = false;

            if(fabs(vertical_rate) <= 10000)
                cur_target->vertical_rate_valid = true;
            else
                cur_target->vertical_rate_valid = false;

            cur_target->time_stamp = QDateTime::currentDateTime().toTime_t();

            if(cur_target->lat_valid && cur_target->lon_valid)
            {
                double dif_lat = lat*M_PI/180.;
                double dif_lon = ((lon*M_PI/180.)-(m_lon*M_PI/180.))*cos(((m_lat+lat)/2.)*M_PI/180.);
                double R = 6371.;

                dif_lat =  dif_lat - (m_lat*M_PI/180.);

                cur_target->rng = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
                cur_target->rng += 1.2; //jangan terlalu mirip
                qreal bearing = atan2(dif_lon,dif_lat)*180./M_PI;

                while(bearing < 0.0)
                {
                    bearing += 360.0;
                }
                cur_target->brn = bearing+2.1; //jangan terlalu mirip
            }
            /*
            qDebug()<<Q_FUNC_INFO<<"icao"<<icao<<m_lat<<m_lon<<cur_target->rng<<cur_target->brn;
            qDebug()<<Q_FUNC_INFO<<"cur_target->call_sign"<<cur_target->call_sign<<call_sign.size();
            qDebug()<<Q_FUNC_INFO<<"cur_target->lat"<<cur_target->lat;
            qDebug()<<Q_FUNC_INFO<<"cur_target->lon"<<cur_target->lon;
            qDebug()<<Q_FUNC_INFO<<"cur_target->alt"<<cur_target->alt;
            qDebug()<<Q_FUNC_INFO<<"cur_target->speed"<<cur_target->speed;
            qDebug()<<Q_FUNC_INFO<<"cur_target->course"<<cur_target->course;
            qDebug()<<Q_FUNC_INFO<<"cur_target->vertical_rate"<<cur_target->vertical_rate;
            qDebug()<<Q_FUNC_INFO<<"cur_target->ground"<<cur_target->ground;
            qDebug()<<Q_FUNC_INFO<<"cur_target->country"<<cur_target->country<<country.size();
            qDebug()<<Q_FUNC_INFO<<"cur_target->time_stamp"<<cur_target->time_stamp;
            */

            if((cur_target->rng < 60.) && ((cur_target->alt < 19000.) && (cur_target->alt > 500.))
                    && (cur_target->lat_valid)  && (cur_target->alt_valid))
            {
                targetListMap.insert(icao,cur_target);
                cur_targets_icao.append(icao);
            }
        }
    }
    return cur_targets_icao;

//    qDebug()<<Q_FUNC_INFO<<"targetListMap->size"<<targetListMap.size();
}

void ADSBDecoder::setLatLon(double lat,double lon)
{
//    qDebug()<<Q_FUNC_INFO<<"lat"<<lat<<"lon"<<lon;
    m_lat = lat;
    m_lon = lon;
}

ADSBParser::ADSBParser()
{
    appendString.clear();
    start_append = false;
    stop_append = false;
}

QJsonArray ADSBParser::parseData(QByteArray json_data)
{
    QByteArray data = preParsedData(json_data);
    QJsonArray arry_json;
    m_error = "";

//    qDebug()<<Q_FUNC_INFO<<data;
    if(!data.isEmpty())
    {
        QJsonParseError error;
        arry_json = QJsonDocument::fromJson(data,&error).array();
        if(error.error != QJsonParseError::NoError)
            m_error = error.errorString();
        qDebug()<<Q_FUNC_INFO<<m_error<<arry_json.size();;

    }
//    qDebug()<<Q_FUNC_INFO<<arry_json.size()<<data;

    return arry_json;
}

QByteArray ADSBParser::preParsedData(QByteArray data)
{
    char buffer = '\0';
    QByteArray ret_string;
    ret_string.clear();

    for(int i=0; i<data.size(); i++)
    {
        buffer = data.at(i);

        if(buffer == '[')
            start_append = true;

        if(buffer == ']' && start_append)
            stop_append = true;

        if(start_append)
            appendString.append(buffer);

        if(start_append && stop_append)
        {
            start_append = false;
            stop_append = false;

//            qDebug()<<Q_FUNC_INFO<<"appendString"<<appendString;
            ret_string = appendString;
            appendString.clear();

        }
    }

    return ret_string;
}

ADSBTargetData::ADSBTargetData()
{
    icao = -1;
    number = -1;
    strncpy(call_sign,"@@@@@@@@@@",10);
    lat = 1000;
    lon = 1000;
    rng = -1.;
    brn = -1.;
    alt = 1000000;
    speed = 10000;
    course = 1000;
    vertical_rate = 10000;
    ground = 10;
    strncpy(country,"@@@@@@@@@@",10);
    QDateTime time = QDateTime::currentDateTime();
    time_stamp = time.toTime_t();
//    qDebug()<<Q_FUNC_INFO<<"time_stamp"<<time_stamp;
}
