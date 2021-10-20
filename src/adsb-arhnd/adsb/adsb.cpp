#include "adsb.h"

#include <QDebug>
#include <QDateTime>
#include <math.h>

using namespace AdsbArhnd;

ADSBDecoder::ADSBDecoder()
{
//    cur_targets_icao.clear();
    cur_targets_icao_from_adsb.clear();
//    cur_targets_icao_from_iff.clear();
    targetListMap.clear();
}

void ADSBDecoder::setTargetNumber(int icao, int number)
{
    ADSBTargetData* cur_target = targetListMap.value(icao);
    cur_target->number = number;
    targetListMap.insert(icao,cur_target);
}

void ADSBDecoder::setTargetIdentity(const QString squawk, const quint8 identity)
{
    ADSBTargetData *target = nullptr;
    foreach (ADSBTargetData* cur_target, targetListMap)
    {
        if(cur_target->squawk_code == squawk)
        {
            target = targetListMap.take(cur_target->icao);
            break;
        }
    }
    if(target)
    {
        target->identity = identity;
        targetListMap.insert(target->icao,target);
    }
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
    cur_targets_icao_from_adsb.remove(icao);
//    qDebug()<<Q_FUNC_INFO<<icao;
}

QList<int> ADSBDecoder::setTargetFromIFF(ADSBTargetData track)
{
    QList<int> cur_targets_icao;
    ADSBTargetData *cur_target;

    if(targetListMap.contains(track.icao))
        cur_target = targetListMap.take(track.icao);
    else
        cur_target = new ADSBTargetData();

    if(!cur_targets_icao_from_adsb.contains(track.icao))
    {
        cur_target->icao = track.icao;
        cur_target->lat = track.lat;
        cur_target->lon = track.lon;
        cur_target->rng = track.rng;
        cur_target->brn = track.brn;
        cur_target->alt = track.alt; //jangan terlalu mirip
        cur_target->speed = track.speed+1.3; //jangan terlalu mirip
        cur_target->course = track.course+1.3; //jangan terlalu mirip
        cur_target->vertical_rate = track.vertical_rate;
        cur_target->ground = track.ground;
        cur_target->lat_valid = track.lat_valid;
        cur_target->lon_valid = track.lon_valid;
        cur_target->alt_valid = track.alt_valid;
        cur_target->speed_valid = track.speed_valid;
        cur_target->course_valid = track.course_valid;
        cur_target->vertical_rate_valid = track.vertical_rate_valid;
        cur_target->time_stamp = track.time_stamp;
        cur_target->identity = track.identity;
        cur_target->squawk_code = track.squawk_code;

//        targetListMap.insert(track.icao,cur_target);
//        cur_targets_icao.append(track.icao);
//        cur_targets_icao_from_iff.insert(track.icao);
//        if(!cur_targets_icao_from_iff.contains(track.icao)) cur_targets_icao_from_iff.append(track.icao);

    }
    else
    {
//        cur_target = targetListMap.take(track.icao);
        cur_target->identity = track.identity;
    }
    targetListMap.insert(track.icao,cur_target);
    cur_targets_icao.append(track.icao);

    /*
    qDebug()<<Q_FUNC_INFO<<"ptr"<<targetListMap.value(track.icao);
    qDebug()<<Q_FUNC_INFO<<"icao"<<targetListMap.value(track.icao)->icao;
    qDebug()<<Q_FUNC_INFO<<"lat"<<targetListMap.value(track.icao)->lat;
    qDebug()<<Q_FUNC_INFO<<"lon"<<targetListMap.value(track.icao)->lon;
    qDebug()<<Q_FUNC_INFO<<"rng"<<targetListMap.value(track.icao)->rng;
    qDebug()<<Q_FUNC_INFO<<"brn"<<targetListMap.value(track.icao)->brn;
    qDebug()<<Q_FUNC_INFO<<"speed"<<targetListMap.value(track.icao)->speed;
    qDebug()<<Q_FUNC_INFO<<"course"<<targetListMap.value(track.icao)->course;
    */

    return cur_targets_icao;
}

QList<int> ADSBDecoder::decode(QJsonArray targets)
{
//    cur_targets_icao.clear();
    QList<int> cur_targets_icao;

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
            QString squawk = target.value("squ").toString("");
            QString cat = target.value("cat").toString("");
            float lat = target.value("lat").toDouble(1000);
            float lon = target.value("lon").toDouble(1000);
//            float alt = 2000.; //tes
            float alt = target.value("alt").toDouble(1000000);
            float speed = target.value("spd").toDouble(10000);
            float course = target.value("trk").toDouble(1000);
            float vertical_rate = target.value("vrt").toDouble(10000);
            quint8 ground = target.value("gda").toString("G").contains("G") ? 0 : 1;
            QString country = target.value("cou").toString("");

            cur_target->icao = icao;
            cur_target->squawk_code = squawk;
            cur_target->cat = decideCat(cat);
            cur_target->trimmed_call_sign = call_sign;
            strncpy(cur_target->call_sign,call_sign.toUtf8().constData(),call_sign.size());
            cur_target->lat = lat;
            cur_target->lon = lon;
//            cur_target->alt = 2000.; //test
            cur_target->alt = alt+150.; //jangan terlalu mirip
            cur_target->speed = speed+1.3; //jangan terlalu mirip
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
                cur_target->rng += 0.2; //jangan terlalu mirip
                qreal bearing = atan2(dif_lon,dif_lat)*180./M_PI;

                while(bearing < 0.0)
                {
                    bearing += 360.0;
                }
                cur_target->brn = bearing+1.1; //jangan terlalu mirip
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
            qDebug()<<Q_FUNC_INFO<<"cur_target->lat_valid"<<cur_target->lat_valid;
            qDebug()<<Q_FUNC_INFO<<"cur_target->alt_valid"<<cur_target->alt_valid;
            qDebug()<<Q_FUNC_INFO<<"squawk_code"<<cur_target->squawk_code;
            qDebug()<<Q_FUNC_INFO<<"cur_target->cat"<<cur_target->cat;
            qDebug()<<Q_FUNC_INFO<<"cat"<<cat;
            */

            bool pass = (cur_target->rng < 100.) && ((cur_target->alt < 26500.) && (cur_target->alt > 1500.))
                    && (cur_target->lat_valid)  && (cur_target->alt_valid);

//            if(icao == 11260195)
//            {
//                pass = true;
//                cur_target->alt = 2200.;
//                cur_target->alt_valid = true;
//            }
            //jarak max 100Km, ketinggian max 9 Km (~ 30000 ft), ketinggian min 500m (~ 1500 ft)
            if(pass)
            {
//                qDebug()<<Q_FUNC_INFO<<"filter pass"<<"squawk_code"<<cur_target->squawk_code;

                targetListMap.insert(icao,cur_target);
                cur_targets_icao.append(icao);
                cur_targets_icao_from_adsb.insert(icao);
//                if(!cur_targets_icao_from_adsb.contains(icao)) cur_targets_icao_from_adsb.append(icao);
            }
        }
    }
    return cur_targets_icao;

//    qDebug()<<Q_FUNC_INFO<<"targetListMap->size"<<targetListMap.size();
}

ADSBTargetData::AircraftCategory ADSBDecoder::decideCat(const QString cat_str)
{
    if(cat_str.contains("A0") || cat_str.contains("A1") || cat_str.contains("A2") || cat_str.contains("A3") || cat_str.contains("A4") || cat_str.contains("A5") || cat_str.contains("A6") || cat_str.contains("F2") || cat_str.contains("F8") || cat_str.contains("F9"))
        return ADSBTargetData::AircraftCategory::FWD;
    else if(cat_str.contains("A7") || cat_str.contains("F3"))
        return ADSBTargetData::AircraftCategory::RWD;
    else if(cat_str.contains("B1") || cat_str.contains("B4") || cat_str.contains("B6") || cat_str.contains("F1") || cat_str.contains("F6") || cat_str.contains("F7") || cat_str.contains("F13"))
        return ADSBTargetData::AircraftCategory::UAV;
    else
        return ADSBTargetData::AircraftCategory::UNKNOWN;
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
    squawk_code = "0000";
    identity = 0;
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
