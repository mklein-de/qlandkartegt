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
    _key_ = QString("%1%2").arg(timestamp).arg(name);
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

    //CTrack * track = tracks[key];
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
