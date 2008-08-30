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
#include "CMainWindow.h"
#include "COverlayDB.h"
#include "CTrackDB.h"
#include "CTrack.h"

#include <QtGui>

bool operator==(const XY& p1, const XY& p2)
{
    return (p1.u == p2.u) && (p1.v == p2.v);
}

COverlayDistance::COverlayDistance(const QList<XY>& pts, QObject * parent)
: IOverlay(parent, "Distance", QPixmap(":/icons/iconDistance16x16"))
, points(pts)
, thePoint(0)
, distance(0)
, doSpecialCursor(false)
, doMove(false)
{

    rectDel  = QRect(0,0,16,16);
    rectMove = QRect(32,0,16,16);
    rectAdd1 = QRect(0,32,16,16);
    rectAdd2 = QRect(32,32,16,16);

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
    IMap& map = CMapDB::self().getMap();
    QList<XY>::iterator p = points.begin();

    thePoint = 0;

    double dist = 900.0;
    while(p != points.end()){
        XY pt1 = *p;
        map.convertRad2Pt(pt1.u, pt1.v);

        double d = (pt.x() - pt1.u) * (pt.x() - pt1.u) + (pt.y() - pt1.v) * (pt.y() - pt1.v);
        if(d < dist){
            thePoint = &(*p);
            dist = d;
        }
        ++p;
    }

    return (dist != 900.0);
}

void COverlayDistance::mouseMoveEvent(QMouseEvent * e)
{

    IMap& map   = CMapDB::self().getMap();
    QPoint pos  = e->pos();
    QPoint pos1 = e->pos();

    if(thePoint){
        XY pt = *thePoint;
        map.convertRad2Pt(pt.u, pt.v);
        pos1 -= QPoint(pt.u - 24, pt.v - 24);
    }


    if(thePoint){
        if(doMove){
            XY pt;
            pt.u = pos.x();
            pt.v = pos.y();
            map.convertPt2Rad(pt.u, pt.v);
            *thePoint = pt;
            theMainWindow->getCanvas()->update();
        }
        else if(rectMove.contains(pos1) || rectDel.contains(pos1) || rectAdd1.contains(pos1) || rectAdd2.contains(pos1)){
            if(!doSpecialCursor){
                QApplication::setOverrideCursor(Qt::PointingHandCursor);
                doSpecialCursor = true;
            }
        }
        else{
            if(doSpecialCursor){
                QApplication::restoreOverrideCursor();
                doSpecialCursor = false;
            }
        }
    }
}

void COverlayDistance::mousePressEvent(QMouseEvent * e)
{
    if(thePoint == 0) return;

    QPoint pos1 = e->pos();
    IMap& map   = CMapDB::self().getMap();

    XY pt = *thePoint;
    map.convertRad2Pt(pt.u, pt.v);
    pos1 -= QPoint(pt.u - 24, pt.v - 24);

    if(rectDel.contains(pos1)){
        int idx = points.indexOf(*thePoint);

        if(idx == -1) return;

        points.takeAt(idx);

        if(points.isEmpty()){
            QStringList keys(key);
            COverlayDB::self().delOverlays(keys);
        }
    }
    else if(rectMove.contains(pos1)){
        doMove = true;
    }
    else if(rectAdd1.contains(pos1)){
        int idx = points.indexOf(*thePoint);

        if(idx == -1) return;

        XY pt;
        pt.u = e->pos().x();
        pt.v = e->pos().y();
        map.convertPt2Rad(pt.u, pt.v);
        points.insert(idx,pt);

        thePoint = &points[idx];
        doMove   = true;

        theMainWindow->getCanvas()->update();
    }
    else if(rectAdd2.contains(pos1)){
        int idx = points.indexOf(*thePoint);

        if(idx == -1) return;

        idx++;

        XY pt;
        pt.u = e->pos().x();
        pt.v = e->pos().y();
        map.convertPt2Rad(pt.u, pt.v);
        points.insert(idx,pt);

        thePoint = &points[idx];
        doMove   = true;

        theMainWindow->getCanvas()->update();

    }

}

void COverlayDistance::mouseReleaseEvent(QMouseEvent * e)
{
    QPoint pos = e->pos();
    IMap& map  = CMapDB::self().getMap();

    if(doMove && thePoint){
        XY pt;
        pt.u = pos.x();
        pt.v = pos.y();
        map.convertPt2Rad(pt.u, pt.v);
        *thePoint = pt;

        calcDistance();
        theMainWindow->getCanvas()->update();

    }

    doMove = false;
    emit sigChanged();
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

    if(thePoint && !doMove){
        pt2 = *thePoint;
        map.convertRad2Pt(pt2.u, pt2.v);
        p.drawPixmap(pt2.u - 5, pt2.v - 5, QPixmap(":/icons/bullet_red.png"));

        p.save();
        p.translate(pt2.u - 24, pt2.v - 24);
        p.drawPixmap(rectDel, QPixmap(":/icons/iconClear16x16.png"));
        p.drawPixmap(rectMove, QPixmap(":/icons/iconWptMove16x16.png"));
        p.drawPixmap(rectAdd1, QPixmap(":/icons/iconAdd16x16.png"));
        p.drawPixmap(rectAdd2, QPixmap(":/icons/iconAdd16x16.png"));

        p.restore();
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

void COverlayDistance::customMenu(QMenu& menu)
{
    menu.addAction(QPixmap(":/icons/iconTrack16x16.png"),tr("Make Track"),this,SLOT(slotToTrack()));
}

void COverlayDistance::slotToTrack()
{

    IMap& map       = CMapDB::self().getDEM();
    CTrack * track  = new CTrack(&CTrackDB::self());

    XY point;
    foreach(point, points){
        CTrack::pt_t pt;
        pt.lon = point.u * RAD_TO_DEG;
        pt.lat = point.v * RAD_TO_DEG;
        pt.ele = map.getElevation(point.u, point.v);

        *track << pt;
    }
    CTrackDB::self().addTrack(track, false);
}


