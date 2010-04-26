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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

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
#include "CRouteDB.h"
#include "CRoute.h"
#include "CDlgEditDistance.h"
#include "CMegaMenu.h"
#include "CDlgConvertToTrack.h"

#include <QtGui>

bool operator==(const XY& p1, const XY& p2)
{
    return (p1.u == p2.u) && (p1.v == p2.v);
}


COverlayDistance::COverlayDistance(const QString& name, const QString& comment, const QList<XY>& pts, QObject * parent)
: IOverlay(parent, "Distance", QPixmap(":/icons/iconDistance16x16"))
, points(pts)
, thePoint(0)
, name(name)
, comment(comment)
, distance(0)
, doSpecialCursor(false)
, doMove(false)
, doFuncWheel(false)
{

    rectDel  = QRect(0,0,16,16);
    rectMove = QRect(32,0,16,16);
    rectAdd1 = QRect(0,32,16,16);
    rectAdd2 = QRect(32,32,16,16);

    calcDistance();
}


COverlayDistance::~COverlayDistance()
{

}


void COverlayDistance::save(QDataStream& s)
{
    s << name << comment << points.size();
    XY pt;
    foreach(pt, points)
    {
        s << pt.u << pt.v;
    }

}


void COverlayDistance::load(QDataStream& s)
{
    XY pt;
    int size;

    points.clear();

    s >> name >> comment >> size;
    for(int i = 0; i < size; ++i)
    {
        s >> pt.u >> pt.v;
        points << pt;
    }

}


QString COverlayDistance::getInfo()
{
    QString info;
    QString val, unit;

    if(!name.isEmpty())
    {
        info += name + "\n";
    }
    if(!comment.isEmpty())
    {
        info += comment + "\n";
    }

    IUnit::self().meter2distance(distance, val, unit);
    info += tr("Length: %1 %2").arg(val).arg(unit);
    return info;
}


bool COverlayDistance::isCloseEnought(const QPoint& pt)
{
    IMap& map = CMapDB::self().getMap();
    QList<XY>::iterator p = points.begin();

    if(doFuncWheel)
    {
        return true;
    }

    thePoint = 0;

    double ref  = doFuncWheel ? (35.0 * 35.0) : (8.0 * 8.0);
    double dist = ref;
    while(p != points.end())
    {
        XY pt1 = *p;
        map.convertRad2Pt(pt1.u, pt1.v);

        double d = (pt.x() - pt1.u) * (pt.x() - pt1.u) + (pt.y() - pt1.v) * (pt.y() - pt1.v);
        if(d < dist)
        {
            thePoint = &(*p);

            if(p != points.begin())
            {
                XY p1 = *p;
                XY p2 = *(p - 1);

                map.convertRad2M(p1.u, p1.v);
                map.convertRad2M(p2.u, p2.v);

                anglePrev = atan2((p2.v - p1.v) , (p2.u - p1.u)) * 180/PI;
            }
            else
            {
                anglePrev = 1000;
            }
            if((p + 1) != points.end())
            {
                XY p1 = *p;
                XY p2 = *(p + 1);

                map.convertRad2M(p1.u, p1.v);
                map.convertRad2M(p2.u, p2.v);

                angleNext = atan2((p2.v - p1.v) , (p2.u - p1.u)) * 180/PI;

            }
            else
            {
                angleNext = 1000;
            }

            dist = d;
        }
        ++p;
    }

    return (dist != ref);
}


void COverlayDistance::mouseMoveEvent(QMouseEvent * e)
{

    IMap& map   = CMapDB::self().getMap();
    QPoint pos  = e->pos();
    QPoint pos1 = e->pos();

    if(thePoint)
    {
        XY pt = *thePoint;
        map.convertRad2Pt(pt.u, pt.v);
        pos1 -= QPoint(pt.u - 24, pt.v - 24);

        if(doMove)
        {
            XY pt;
            pt.u = pos.x();
            pt.v = pos.y();
            map.convertPt2Rad(pt.u, pt.v);
            *thePoint = pt;
            theMainWindow->getCanvas()->update();
        }
        else if(rectMove.contains(pos1) || rectDel.contains(pos1) || rectAdd1.contains(pos1) || rectAdd2.contains(pos1))
        {
            if(!doSpecialCursor)
            {
                QApplication::setOverrideCursor(Qt::PointingHandCursor);
                doSpecialCursor = true;
            }
        }
        else
        {
            if(doSpecialCursor)
            {
                QApplication::restoreOverrideCursor();
                doSpecialCursor = false;
            }
        }
    }
}


void COverlayDistance::mousePressEvent(QMouseEvent * e)
{
    if(thePoint == 0) return;

    if(e->button() == Qt::LeftButton)
    {
        if(doMove)
        {
            doMove      = false;
            thePoint    = 0;
            calcDistance();
            theMainWindow->getCanvas()->update();

            QApplication::restoreOverrideCursor();

            emit sigChanged();
            return;
        }

        if(!doFuncWheel)
        {
            doFuncWheel = true;
            theMainWindow->getCanvas()->update();
            return;
        }

        QPoint pos1 = e->pos();
        IMap& map   = CMapDB::self().getMap();

        XY pt = *thePoint;
        map.convertRad2Pt(pt.u, pt.v);
        pos1 -= QPoint(pt.u - 24, pt.v - 24);

        if(rectDel.contains(pos1))
        {
            int idx = points.indexOf(*thePoint);

            if(idx == -1) return;

            points.takeAt(idx);

            if(points.isEmpty())
            {
                QStringList keys(key);
                COverlayDB::self().delOverlays(keys);
            }
            calcDistance();
            doFuncWheel = false;
            thePoint    = 0;

            emit sigChanged();
        }
        else if(rectMove.contains(pos1))
        {
            QApplication::setOverrideCursor(QCursor(QPixmap(":/cursors/cursorMoveWpt"),0,0));
            savePoint   = *thePoint;
            doMove      = true;
            doFuncWheel = false;
        }
        else if(rectAdd1.contains(pos1))
        {
            int idx = points.indexOf(*thePoint);

            if(idx == -1) return;

            XY pt;
            pt.u = e->pos().x();
            pt.v = e->pos().y();
            map.convertPt2Rad(pt.u, pt.v);
            points.insert(idx,pt);

            thePoint    = &points[idx];
            doMove      = true;
            doFuncWheel = false;

            theMainWindow->getCanvas()->update();
        }
        else if(rectAdd2.contains(pos1))
        {
            int idx = points.indexOf(*thePoint);

            if(idx == -1) return;

            idx++;

            XY pt;
            pt.u = e->pos().x();
            pt.v = e->pos().y();
            map.convertPt2Rad(pt.u, pt.v);
            points.insert(idx,pt);

            thePoint    = &points[idx];
            doMove      = true;
            doFuncWheel = false;

            theMainWindow->getCanvas()->update();

        }
        else
        {
            doFuncWheel = false;
            thePoint    = 0;
        }
    }
    else if(e->button() == Qt::RightButton)
    {
        if(doMove)
        {
            doMove      = false;

            *thePoint   = savePoint;
            thePoint    = 0;

            calcDistance();
            theMainWindow->getCanvas()->update();

            QApplication::restoreOverrideCursor();

            emit sigChanged();
            return;
        }

    }
}


void COverlayDistance::mouseReleaseEvent(QMouseEvent * e)
{
    //     QPoint pos = e->pos();
    //     IMap& map  = CMapDB::self().getMap();
    //
    //     emit sigChanged();
}


void COverlayDistance::draw(QPainter& p)
{
    if(points.isEmpty()) return;

    IMap& map = CMapDB::self().getMap();

    QPixmap icon(":/icons/bullet_blue.png");
    XY pt1, pt2;
    pt1 = points.first();
    map.convertRad2Pt(pt1.u, pt1.v);

    for(int i = 1; i < points.count(); i++)
    {
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

    for(int i = 0; i < points.count(); i++)
    {
        pt2 = points[i];
        map.convertRad2Pt(pt2.u, pt2.v);
        p.drawPixmap(pt2.u - 5, pt2.v - 5, icon);
    }

    if(thePoint && !doMove)
    {
        pt2 = *thePoint;
        map.convertRad2Pt(pt2.u, pt2.v);

        if(doFuncWheel)
        {

            p.setPen(QColor(100,100,255,200));
            p.setBrush(QColor(255,255,255,200));
            p.drawEllipse(pt2.u - 35, pt2.v - 35, 70, 70);

            p.drawPixmap(pt2.u - 5, pt2.v - 5, QPixmap(":/icons/bullet_red.png"));

            p.save();
            p.translate(pt2.u - 24, pt2.v - 24);
            p.drawPixmap(rectDel, QPixmap(":/icons/iconClear16x16.png"));
            p.drawPixmap(rectMove, QPixmap(":/icons/iconMove16x16.png"));
            if(anglePrev < 360)
            {
                p.save();
                p.setRenderHint(QPainter::SmoothPixmapTransform, true);
                p.translate(rectAdd1.center());
                p.rotate(-anglePrev);
                p.drawPixmap(QPoint(-8, -8), QPixmap(":/icons/iconAddPoint16x16.png"));
                p.restore();
            }
            else
            {
                p.save();
                p.setRenderHint(QPainter::SmoothPixmapTransform, true);
                p.translate(rectAdd1.center());
                p.rotate(-angleNext + 180);
                p.drawPixmap(QPoint(-8, -8), QPixmap(":/icons/iconAddPoint16x16.png"));
                p.restore();
            }

            if(angleNext < 360)
            {
                p.save();
                p.setRenderHint(QPainter::SmoothPixmapTransform, true);
                p.translate(rectAdd2.center());
                p.rotate(-angleNext);
                p.drawPixmap(QPoint(-8, -8), QPixmap(":/icons/iconAddPoint16x16.png"));
                p.restore();
            }
            else
            {
                p.save();
                p.setRenderHint(QPainter::SmoothPixmapTransform, true);
                p.translate(rectAdd2.center());
                p.rotate(-anglePrev + 180);
                p.drawPixmap(QPoint(-8, -8), QPixmap(":/icons/iconAddPoint16x16.png"));
                p.restore();
            }

            p.restore();
        }
        else
        {
            p.setPen(QColor(100,100,255,200));
            p.setBrush(QColor(255,255,255,200));
            p.drawEllipse(pt2.u - 8, pt2.v - 8, 16, 16);
        }
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

    if(points.count() < 2)
    {
        return;
    }

    double a1,a2;
    XY pt1, pt2;
    pt1 = points.first();

    for(int i = 1; i < points.count(); i++)
    {
        pt2 = points[i];

        distance += ::distance(pt1, pt2, a1, a2);

        pt1 = pt2;
    }

}


void COverlayDistance::customMenu(QMenu& menu)
{
    menu.addAction(QPixmap(":/icons/iconTrack16x16.png"),tr("Make Track"),this,SLOT(slotToTrack()));
    menu.addAction(QPixmap(":/icons/iconRoute16x16.png"),tr("Make Route"),this,SLOT(slotToRoute()));
    menu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit..."),this,SLOT(slotEdit()));
}


void COverlayDistance::slotToTrack()
{
    if(points.isEmpty()) return;

    IMap& map       = CMapDB::self().getDEM();
    CTrack * track  = new CTrack(&CTrackDB::self());

    track->name = name;

    double dist, d, delta = 10.0, a1 , a2;
    XY pt1, pt2, ptx;
    CTrack::pt_t pt;

    CDlgConvertToTrack dlg(0);
    if(dlg.exec() == QDialog::Rejected)
    {
        return;
    }

    delta = dlg.getDelta();

    if(delta == -1)
    {

        for(int i = 0; i < points.count(); ++i)
        {
            pt2 = points[i];
            pt.lon = pt2.u * RAD_TO_DEG;
            pt.lat = pt2.v * RAD_TO_DEG;
            pt.ele = map.getElevation(pt2.u, pt2.v);
            *track << pt;
        }
    }
    else
    {
        if((distance / delta) > (MAX_TRACK_SIZE - points.count()))
        {
            delta = distance / (MAX_TRACK_SIZE - points.count());
        }

        // 1st point
        pt1 = points.first();
        pt.lon = pt1.u * RAD_TO_DEG;
        pt.lat = pt1.v * RAD_TO_DEG;
        pt.ele = map.getElevation(pt1.u, pt1.v);
        *track << pt;

        // all other points
        for(int i = 1; i < points.count(); ++i)
        {
            pt2 = points[i];

            // all points from pt1 -> pt2, with 10m steps
            dist = ::distance(pt1, pt2, a1, a2);
            a1 *= DEG_TO_RAD;

            d = delta;
            while(d < dist)
            {
                ptx = GPS_Math_Wpt_Projection(pt1, d, a1);
                pt.lon = ptx.u * RAD_TO_DEG;
                pt.lat = ptx.v * RAD_TO_DEG;
                pt.ele = map.getElevation(ptx.u, ptx.v);
                *track << pt;

                d += delta;
            }

            // and finally the next point
            pt.lon = pt2.u * RAD_TO_DEG;
            pt.lat = pt2.v * RAD_TO_DEG;
            pt.ele = map.getElevation(pt2.u, pt2.v);
            *track << pt;

            pt1 = pt2;
        }
    }
    CTrackDB::self().addTrack(track, false);

    CMegaMenu::self().switchByKeyWord("Tracks");
}


void COverlayDistance::slotToRoute()
{
    if(points.isEmpty()) return;

    CRoute * route  = new CRoute(&CRouteDB::self());

    route->setName(name);

    XY pt;

    for(int i = 0; i < points.count(); ++i)
    {
        pt = points[i];
        pt.u = pt.u * RAD_TO_DEG;
        pt.v = pt.v * RAD_TO_DEG;
        route->addPosition(pt.u, pt.v);
    }

    CRouteDB::self().addRoute(route, false);

    CMegaMenu::self().switchByKeyWord("Routes");
}


void COverlayDistance::slotEdit()
{
    CDlgEditDistance dlg(*this, 0);
    dlg.exec();

    emit sigChanged();
}


void COverlayDistance::makeVisible()
{
    double north =  -90.0 * DEG_TO_RAD;
    double south =  +90.0 * DEG_TO_RAD;
    double west  = +180.0 * DEG_TO_RAD;
    double east  = -180.0 * DEG_TO_RAD;

    XY pt;
    foreach(pt, points)
    {
        if(pt.u < west)  west  = pt.u;
        if(pt.u > east)  east  = pt.u;
        if(pt.v < south) south = pt.v;
        if(pt.v > north) north = pt.v;
    }
    CMapDB::self().getMap().zoom(west, north, east, south);
}


void COverlayDistance::looseFocus()
{
    if(doSpecialCursor)
    {
        QApplication::restoreOverrideCursor();
        doSpecialCursor = false;
    }
    doMove          = false;
    doFuncWheel     = false;
}


QRectF COverlayDistance::getBoundingRectF()
{
    double north =  -90.0 * DEG_TO_RAD;
    double south =  +90.0 * DEG_TO_RAD;
    double west  = +180.0 * DEG_TO_RAD;
    double east  = -180.0 * DEG_TO_RAD;

    XY pt;
    foreach(pt, points)
    {
        if(pt.u < west)  west  = pt.u;
        if(pt.u > east)  east  = pt.u;
        if(pt.v < south) south = pt.v;
        if(pt.v > north) north = pt.v;
    }

    return QRectF(QPointF(west * RAD_TO_DEG,north * RAD_TO_DEG),QPointF(east * RAD_TO_DEG,south * RAD_TO_DEG));

}
