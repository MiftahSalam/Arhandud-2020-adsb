/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AIS Decoder Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */

#ifndef ADSB_H
#define ADSB_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include <QSet>

#include "adsb-arhnd_global.h"

const quint64 ADSB_TARGET_EXPIRED = 25;

namespace AdsbArhnd {
class ADSB_ARHND_EXPORT ADSBTargetData
{
public:
    ADSBTargetData();

    int icao;
    int number;
    char call_sign[11];
    QString trimmed_call_sign;
    float lat; //deg
    float lon; //deg
    float rng; //km
    float brn; //deg
    float alt; //feet
    float speed; //kts
    float course; //deg
    float vertical_rate;
    bool lat_valid;
    bool lon_valid;
    bool alt_valid;
    bool speed_valid;
    bool course_valid;
    bool vertical_rate_valid;
    bool selected;
    quint8 ground;
    quint8 identity;
    char country[11];
    QString trimmed_country;
    QString squawk_code;
    uint time_stamp;
};

class ADSB_ARHND_EXPORT ADSBParser
{
public:
    ADSBParser();

    QJsonArray parseData(QByteArray json_data);
    QString getError() { return m_error; }

private:
    QByteArray preParsedData(QByteArray data);

    QByteArray appendString;
    bool start_append;
    bool stop_append;
    QString m_error;

};

class ADSB_ARHND_EXPORT ADSBDecoder
{
public:
    ADSBDecoder();

    QHash<int,ADSBTargetData*> getTargets() { return targetListMap; }
    ADSBTargetData* getTarget(int icao) const { return targetListMap.value(icao); }
    void setTargetNumber(int icao, int number);
    void setTargetIdentity(const QString squawk, const quint8 identity);
    void updateADSB();
    void setLatLon(double lat,double lon);
    QList<int> setTargetFromIFF(ADSBTargetData track);
    QList<int> decode(QJsonArray targets);

private:
    QSet<int> cur_targets_icao_from_adsb;
    QHash<int,ADSBTargetData*> targetListMap;

    bool IsExpired(int icao);
    void deleteTarget(int icao);
    double m_lat,m_lon;
};

}

#endif
