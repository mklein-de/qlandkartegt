/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "COverlayDistance.h"
#include "CMapDB.h"
#include "IMap.h"
#include "GeoMath.h"
#include "IUnit.h"

#include <QtGui>

COverlayDistance::COverlayDistance(const QVector<XY>& pts, QObject * parent)
: IOverlay(parent, "Distance", QPixmap(":/icons/iconDistance16x16"))
, points(pts)
, doAddPoints(points.size() == 1)
, distance(0)
{

}

COverlayDistance::~COverlayDistance()
{

}

QString COverlayDistance::getInfo()
{
    QString val, unit;
    IUnit::self().meter2distance(distance, val, unit);
    return tr("Length: %1 %2").arg(val).arg(unit);
}

bool COverlayDistance::isCloseEnought(const QPoint& pt)
{
    return false;
}

void COverlayDistance::draw(QPainter& p)
{
    if(points.isEmpty()) return;

    IMap& map = CMapDB::self().getMap();

    QPixmap icon(":/icons/bullet_blue.png");
    XY pt1, pt2;
    pt1 = points.first();
    map.convertRad2Pt(pt1.u, pt1.v);

    for(int i = 1; i < points.count(); i++){
        pt2 = points[i];
        map.convertRad2Pt(pt2.u, pt2.v);

        p.setPen(QPen(Qt::white, 5));
        p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);
        p.setPen(QPen(Qt::darkBlue, 3));
        p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);
        p.setPen(QPen(Qt::white, 1));
        p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);

        p.drawPixmap(pt2.u - 5, pt2.v - 5, icon);

        pt1 = pt2;
    }

    for(int i = 0; i < points.count(); i++){
        pt2 = points[i];
        map.convertRad2Pt(pt2.u, pt2.v);
        p.drawPixmap(pt2.u - 5, pt2.v - 5, icon);
    }
}

void COverlayDistance::addPoint(XY& pt)
{
    points << pt;
    calcDistance();
    emit sigChanged();
}

void COverlayDistance::calcDistance()
{
    distance = 0.0;

    double a1,a2;
    XY pt1, pt2;
    pt1 = points.first();

    for(int i = 1; i < points.count(); i++){
        pt2 = points[i];

        distance += ::distance(pt1, pt2, a1, a2);

        pt1 = pt2;
    }

}
