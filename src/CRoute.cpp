/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CRoute.h"
#include "WptIcons.h"
#include "GeoMath.h"

#include <QtGui>

quint32 CRoute::keycnt = 0;

struct rte_head_entry_t
{
    rte_head_entry_t() : type(CRoute::eEnd), offset(0) {}
    qint32      type;
    quint32     offset;
    QByteArray  data;
};

QDataStream& operator >>(QDataStream& s, CRoute& route)
{
    quint32 nRtePts = 0;
    QIODevice * dev = s.device();
    qint64      pos = dev->pos();

    char magic[9];
    s.readRawData(magic,9);

    if(strncmp(magic,"QLRte   ",9)) {
        dev->seek(pos);
        return s;
    }

    QList<rte_head_entry_t> entries;

    while(1) {
        rte_head_entry_t entry;
        s >> entry.type >> entry.offset;
        entries << entry;
        if(entry.type == CRoute::eEnd) break;
    }

    QList<rte_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end()) {
        qint64 o = pos + entry->offset;
        dev->seek(o);
        s >> entry->data;

        switch(entry->type) {
            case CRoute::eBase:
            {

                QDataStream s1(&entry->data, QIODevice::ReadOnly);

                s1 >> route._key_;
                s1 >> route.timestamp;
                s1 >> route.name;
                s1 >> route.iconname;
                route.icon = getWptIconByName(route.iconname);
                break;
            }

            case CRoute::eRtePts:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                quint32 n;

                route.routeDegree.clear();
                s1 >> nRtePts;

                for(n = 0; n < nRtePts; ++n) {
                    XY rtept;
                    float u, v;

                    s1 >> u;
                    s1 >> v;

                    rtept.u = u;
                    rtept.v = v;
                    route.routeDegree << rtept;
                }
                break;
            }
            default:;
        }

        ++entry;
    }

    return s;
}


QDataStream& operator <<(QDataStream& s, CRoute& route)
{
    QList<rte_head_entry_t> entries;

    //---------------------------------------
    // prepare base data
    //---------------------------------------
    rte_head_entry_t entryBase;
    entryBase.type = CRoute::eBase;
    QDataStream s1(&entryBase.data, QIODevice::WriteOnly);

    s1 << route.key();
    s1 << route.timestamp;
    s1 << route.name;
    s1 << route.iconname;

    entries << entryBase;

    //---------------------------------------
    // prepare routepoint data
    //---------------------------------------
    rte_head_entry_t entryRtePts;
    entryRtePts.type = CRoute::eRtePts;
    QDataStream s2(&entryRtePts.data, QIODevice::WriteOnly);

    QList<XY>& rtepts           = route.getRoutePoints();
    QList<XY>::iterator rtept   = rtepts.begin();

    s2 << (quint32)rtepts.size();
    while(rtept != rtepts.end()) {
        s2 << (float)rtept->u;
        s2 << (float)rtept->v;
        ++rtept;
    }

    entries << entryRtePts;

    //---------------------------------------
    // prepare terminator
    //---------------------------------------
    rte_head_entry_t entryEnd;
    entryEnd.type = CRoute::eEnd;
    entries << entryEnd;

    //---------------------------------------
    //---------------------------------------
    // now start to actually write data;
    //---------------------------------------
    //---------------------------------------
    // write magic key
    s.writeRawData("QLRte   ",9);

    // calculate offset table
    quint32 offset = entries.count() * 8 + 9;

    QList<rte_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end()) {
        entry->offset = offset;
        offset += entry->data.size() + sizeof(quint32);
        ++entry;
    }

    // write offset table
    entry = entries.begin();
    while(entry != entries.end()) {
        s << entry->type << entry->offset;
        ++entry;
    }

    // write entry data
    entry = entries.begin();
    while(entry != entries.end()) {
        s << entry->data;
        ++entry;
    }

    return s;
}


void operator >>(QFile& f, CRoute& route)
{
    f.open(QIODevice::ReadOnly);
    QDataStream s(&f);
    s >> route;
    f.close();
}


void operator <<(QFile& f, CRoute& route)
{
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
    s << route;
    f.close();
}


CRoute::CRoute(QObject * parent)
: QObject(parent)
, timestamp(QDateTime::currentDateTime().toUTC().toTime_t ())
, name(tr("Route"))
, dist(0.0)
, iconname("Small City")
, icon(getWptIconByName(iconname))
, highlight(false)
, firstTime(true)
{

}


CRoute::~CRoute()
{

}


void CRoute::genKey()
{
    _key_ = QString("%1%2%3").arg(timestamp).arg(name).arg(keycnt++);
}


const QString& CRoute::key()
{
    if(_key_.isEmpty()) genKey();
    return _key_;
}


void CRoute::addPosition(const double lon, const double lat)
{
    XY pt;
    pt.u = lon;
    pt.v = lat;
    routeDegree << pt;

    calcDistance();

    emit sigChanged();
}


void CRoute::calcDistance()
{
    dist = 0.0;
    if(routeDegree.size() < 2) return;

    XY pt1,pt2;
    double a1,a2;

    QList<XY>::const_iterator p1 = routeDegree.begin();
    QList<XY>::const_iterator p2 = routeDegree.begin() + 1;
    while(p2 != routeDegree.end()) {
        pt1.u = p1->u * DEG_TO_RAD; pt1.v = p1->v * DEG_TO_RAD;
        pt2.u = p2->u * DEG_TO_RAD; pt2.v = p2->v * DEG_TO_RAD;
        dist += distance(pt1,pt2,a1,a2);
        ++p2; ++p1;
    }

}


QRectF CRoute::getBoundingRectF()
{

    double north =  -90.0;
    double south =  +90.0;
    double west  = +180.0;
    double east  = -180.0;

    //CRoute * route = routes[key];
    QList<XY>& rtepts = getRoutePoints();
    QList<XY>::const_iterator rtept = rtepts.begin();
    while(rtept != rtepts.end()) {

        if(rtept->u < west)  west  = rtept->u;
        if(rtept->u > east)  east  = rtept->u;
        if(rtept->v < south) south = rtept->v;
        if(rtept->v > north) north = rtept->v;

        ++rtept;
    }

    return QRectF(QPointF(west,north),QPointF(east,south));
}


void CRoute::setIcon(const QString& symname)
{
    iconname = symname;
    icon     = getWptIconByName(iconname);

    emit sigChanged();
}
