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
#include "COverlayDistanceEditWidget.h"
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
#include "CMegaMenu.h"
#include "CDlgConvertToTrack.h"

#include <QtGui>

bool operator==(const XY& p1, const XY& p2)
{
    return (p1.u == p2.u) && (p1.v == p2.v);
}

QPointer<COverlayDistanceEditWidget> overlayDistanceEditWidget;

COverlayDistance::COverlayDistance(const QString& name, const QString& comment, double speed, const QList<pt_t>& pts, QObject * parent)
: IOverlay(parent, "Distance", ":/icons/iconDistance16x16.png")
, points(pts)
, thePoint(0)
, distance(0)
, speed(speed)
, doSpecialCursor(false)
, doMove(false)
, doFuncWheel(false)
, addType(eNone)
, isEdit(false)
{

    if(name.isEmpty())
    {
        setName(tr("Tour %1").arg(keycnt));
    }
    else
    {
        setName(name);
    }

    setComment(comment);

    rectDel  = QRect(0,0,16,16);
    rectMove = QRect(32,0,16,16);
    rectAdd1 = QRect(0,32,16,16);
    rectAdd2 = QRect(32,32,16,16);

    calcDistance();

    if(pts.size() == 1)
    {
        points.append(points[0]);
        thePoint    = &points[1];
        doMove      = true;
        addType     = eAtEnd;
        doFuncWheel = false;
    }
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
    s << speed;
    s << getKey();
}


void COverlayDistance::load(QDataStream& s)
{
    pt_t pt;
    int size;
    QString key;

    points.clear();

    s >> name >> comment >> size;
    for(int i = 0; i < size; ++i)
    {
        s >> pt.u >> pt.v;
        pt.idx = i;
        points << pt;
    }
    s >> speed;
    s >> key;

    setKey(key);

}


QString COverlayDistance::getInfo()
{
    QString info;
    QString val, unit;

    IUnit::self().meter2distance(distance, val, unit);

    if(!name.isEmpty())
    {
        info += name + "\n";
    }
    if(!comment.isEmpty())
    {
        if(comment.length() < 60)
        {
            info += comment + "\n";
        }
        else
        {
            info += comment.left(57) + "...\n";
        }
    }


    info += tr("Length: %1 %2").arg(val).arg(unit);

    if(speed > 0)
    {
        info += "\n" + QString::number(speed * IUnit::self().speedfactor)  + IUnit::self().speedunit + " -> ";

        double ttime = val.toDouble() * 3.6/ (speed * IUnit::self().speedfactor);
        quint32 days = ttime / 86400;

        QTime time;
        time = time.addSecs(ttime);
        if(days)
        {
            info += tr("%1:").arg(days) + time.toString("HH:mm") + "h";
        }
        else
        {
            info += time.toString("HH:mm") + "h";
        }
    }
    return info;
}


bool COverlayDistance::isCloseEnough(const QPoint& pt)
{
    IMap& map = CMapDB::self().getMap();
    QList<pt_t>::iterator p = points.begin();

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

    subline.clear();

    if(thePoint)
    {
        XY pt = *thePoint;
        map.convertRad2Pt(pt.u, pt.v);
        pos1 -= QPoint(pt.u - 24, pt.v - 24);

        if(doMove)
        {
            pt_t pt;
            pt.idx  = thePoint->idx;
            pt.u    = pos.x();
            pt.v    = pos.y();
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

        if(addType != eNone)
        {
            // find subline between last steady point and current point
            double u1, v1, u2, v2;
            IMap& map = CMapDB::self().getMap();

            if(*thePoint == points.last())
            {
                u1 = (points.end() - 2)->u;
                v1 = (points.end() - 2)->v;
            }
            else if(*thePoint == points.first())
            {
                u1 = (points.begin() + 1)->u;
                v1 = (points.begin() + 1)->v;
            }
            else
            {
                int idx = points.indexOf(*thePoint);

                if(addType == eAfter)
                {
                    idx--;
                }
                else
                {
                    idx++;
                }

                u1 = (points.begin() + idx)->u;
                v1 = (points.begin() + idx)->v;
            }

            map.convertRad2Pt(u1,v1);
            QPoint pt1(u1, v1);

            u2 = thePoint->u;
            v2 = thePoint->v;

            map.convertRad2Pt(u2,v2);
            QPoint pt2(u2, v2);


            CMapDB::self().getMap().getClosePolyline(pt1, pt2, 10, leadline);

            if(!leadline.isEmpty())
            {
                GPS_Math_SubPolyline(pt1, pt2, 10, leadline, subline);
            }
        }
    }

}


void COverlayDistance::mousePressEvent(QMouseEvent * e)
{
    if(thePoint == 0) return;
    IMap& map   = CMapDB::self().getMap();

    if(e->button() == Qt::LeftButton)
    {
        if(doMove)
        {
            if(*thePoint == points.last() && addType == eAtEnd)
            {
                const int size = subline.size();
                if(size < 2)
                {
                    pt_t pt;
                    pt.u    = e->pos().x();
                    pt.v    = e->pos().y();
                    map.convertPt2Rad(pt.u, pt.v);

                    points.push_back(pt);
                }
                else
                {
                    pt_t pt;
                    pt.u = subline[1].x();
                    pt.v = subline[1].y();
                    map.convertPt2Rad(pt.u, pt.v);

                    *thePoint = pt;

                    for(int i = 2; i < size; i++)
                    {
                        pt.u = subline[i].x();
                        pt.v = subline[i].y();
                        map.convertPt2Rad(pt.u, pt.v);

                        points.push_back(pt);
                    }

                    points.push_back(pt);

                }
                thePoint = &points.last();
            }
            else if(*thePoint == points.first() && addType == eAtEnd)
            {
                const int size = subline.size();
                if(size < 2)
                {

                    pt_t pt;
                    pt.u = e->pos().x();
                    pt.v = e->pos().y();
                    map.convertPt2Rad(pt.u, pt.v);

                    points.push_front(pt);
                }
                else
                {
                    pt_t pt;
                    pt.u = subline[1].x();
                    pt.v = subline[1].y();
                    map.convertPt2Rad(pt.u, pt.v);

                    *thePoint = pt;

                    for(int i = 2; i < size; i++)
                    {
                        pt.u = subline[i].x();
                        pt.v = subline[i].y();
                        map.convertPt2Rad(pt.u, pt.v);

                        points.push_front(pt);
                    }

                    points.push_front(pt);
                }
                thePoint = &points.first();
            }
            else if(addType != eNone)
            {
                pt_t pt;
                const int size = subline.size();
                int idx = points.indexOf(*thePoint);

                if(size > 2)
                {
                    if(addType == eAfter)
                    {
                        pt.u = subline[0].x();
                        pt.v = subline[0].y();
                        map.convertPt2Rad(pt.u, pt.v);

                        points[idx - 1] = pt;

                        pt.u = subline[size - 1].x();
                        pt.v = subline[size - 1].y();
                        map.convertPt2Rad(pt.u, pt.v);

                        points[idx] = pt;

                        for(int i = 1; i < size - 1; i++)
                        {
                            pt.u = subline[i].x();
                            pt.v = subline[i].y();
                            map.convertPt2Rad(pt.u, pt.v);

                            points.insert(idx - 1 + i,pt);
                        }
                        idx += size - 2;
                    }
                    else
                    {
                        pt.u = subline[0].x();
                        pt.v = subline[0].y();
                        map.convertPt2Rad(pt.u, pt.v);

                        points[idx + 1] = pt;

                        pt.u = subline[size - 1].x();
                        pt.v = subline[size - 1].y();
                        map.convertPt2Rad(pt.u, pt.v);

                        points[idx] = pt;

                        for(int i = 1; i < size - 1; i++)
                        {
                            pt.u = subline[i].x();
                            pt.v = subline[i].y();
                            map.convertPt2Rad(pt.u, pt.v);

                            points.insert(idx+1,pt);
                        }
                    }
                }

                if(addType == eAfter)
                {
                    idx++;
                }


                pt.u = e->pos().x();
                pt.v = e->pos().y();
                map.convertPt2Rad(pt.u, pt.v);
                points.insert(idx,pt);

                thePoint    = &points[idx];
            }
            else
            {
                pt_t pt;
                pt.u = e->pos().x();
                pt.v = e->pos().y();
                map.convertPt2Rad(pt.u, pt.v);

                *thePoint = pt;

                doMove      = false;
                addType     = eNone;
                thePoint    = 0;

            }

            subline.clear();

            calcDistance();
            theMainWindow->getCanvas()->update();

            emit sigChanged();
            return;
        }

        if(!doFuncWheel)
        {
            selectedPoints.clear();
            selectedPoints << points.indexOf(*thePoint);
            emit sigSelectionChanged();

            doFuncWheel = true;
            theMainWindow->getCanvas()->update();
            return;
        }

        QPoint pos1 = e->pos();


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
                QStringList keys(getKey());
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
            doMove      = true;
            doFuncWheel = false;

            savePoint = *thePoint;
        }
        else if(rectAdd1.contains(pos1))
        {
            int idx = points.indexOf(*thePoint);

            if(idx == -1) return;

            pt_t pt;
            pt.u = e->pos().x();
            pt.v = e->pos().y();
            map.convertPt2Rad(pt.u, pt.v);
            points.insert(idx,pt);

            thePoint    = &points[idx];
            doMove      = true;
            addType     = eBefore;
            doFuncWheel = false;

            theMainWindow->getCanvas()->update();
        }
        else if(rectAdd2.contains(pos1))
        {
            int idx = points.indexOf(*thePoint);

            if(idx == -1) return;

            idx++;

            pt_t pt;
            pt.u = e->pos().x();
            pt.v = e->pos().y();
            map.convertPt2Rad(pt.u, pt.v);
            points.insert(idx,pt);

            thePoint    = &points[idx];
            doMove      = true;
            addType     = eAfter;
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

            if(addType != eNone)
            {
                points.removeOne(*thePoint);
            }
            else
            {
                *thePoint = savePoint;
            }
            thePoint = 0;

            calcDistance();
            theMainWindow->getCanvas()->update();
            emit sigChanged();

            doMove      = false;
            addType     = eNone;
            subline.clear();

            COverlayDB::self().emitSigChanged();

            QApplication::restoreOverrideCursor();
            return;
        }
    }

    selectedPoints.clear();
    if(addType == eNone)
    {
        emit sigSelectionChanged();
    }
}


void COverlayDistance::mouseReleaseEvent(QMouseEvent * e)
{

}


void COverlayDistance::draw(QPainter& p)
{
    if(points.isEmpty()) return;

    IMap& map = CMapDB::self().getMap();

    QPixmap icon_blue(":/icons/small_bullet_blue.png");
    QPixmap icon_red(":/icons/small_bullet_red.png");
    QPixmap icon_BigRed(":/icons/bullet_red.png");
    XY pt1, pt2;

    int i;
    int start   = 0;
    int stop    = points.count();
    int skip    = -1;


    // if there is an active subline fine tune start and stop index
    // to make the subline replace the first of last line segment
    if(thePoint && !subline.isEmpty())
    {
        if(*thePoint == points.last() )
        {
            stop -= 1;
        }
        else if(*thePoint == points.first())
        {
            start += 1;
        }


        int idx = points.indexOf(*thePoint);

        if(addType == eAfter)
        {
            skip = idx;
        }
        else
        {
            skip = idx + 1;
        }
    }

    pt1 = points[start];
    map.convertRad2Pt(pt1.u, pt1.v);

    // draw the lines
    for(i = start + 1; i < stop; i++)
    {

        pt2 = points[i];
        map.convertRad2Pt(pt2.u, pt2.v);

        if(i != skip)
        {
            if(highlight)
            {
                p.setPen(QPen(Qt::white, 7));
                p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);
                p.setPen(QPen(Qt::blue, 5));
                p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);
                p.setPen(QPen(Qt::white, 1));
                p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);
            }
            else
            {
                p.setPen(QPen(Qt::white, 5));
                p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);
                p.setPen(QPen(Qt::darkBlue, 3));
                p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);
                p.setPen(QPen(Qt::white, 1));
                p.drawLine(pt1.u, pt1.v, pt2.u, pt2.v);
            }
        }
        pt1 = pt2;
    }

    // draw the points
    for(i = start; i < stop; i++)
    {
        pt2 = points[i];

        map.convertRad2Pt(pt2.u, pt2.v);
        p.drawPixmap(pt2.u - 4, pt2.v - 4, icon_blue);
    }

    // overlay _the_ point with a red bullet
    if(thePoint)
    {
        pt2 = *thePoint;
        map.convertRad2Pt(pt2.u, pt2.v);
        p.drawPixmap(pt2.u - 4, pt2.v - 4, icon_red);
    }


    // if there is a subline draw it
    if(!subline.isEmpty())
    {

        QPen pen;
        pen.setBrush(QBrush(QColor(255,0,255,150)));
        pen.setWidth(20);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);

        p.setPen(pen);
        p.drawPolyline(leadline);

        if(highlight)
        {
            p.setPen(QPen(Qt::white, 7));
            p.drawPolyline(subline);
            p.setPen(QPen(Qt::blue, 5));
            p.drawPolyline(subline);
            p.setPen(QPen(Qt::white, 1));
            p.drawPolyline(subline);
        }
        else
        {
            p.setPen(QPen(Qt::white, 5));
            p.drawPolyline(subline);
            p.setPen(QPen(Qt::darkBlue, 3));
            p.drawPolyline(subline);
            p.setPen(QPen(Qt::white, 1));
            p.drawPolyline(subline);
        }


        p.setPen(Qt::black);
        for(i = 1; i < (subline.size() - 1); i++)
        {
            p.drawPixmap(subline[i] - QPoint(4,4), icon_red);
        }


    }

    if(thePoint && !doMove)
    {
        pt2 = *thePoint;
        map.convertRad2Pt(pt2.u, pt2.v);

        if(doFuncWheel)
        {

            p.setPen(CCanvas::penBorderBlue);
            p.setBrush(CCanvas::brushBackWhite);
            p.drawEllipse(pt2.u - 35, pt2.v - 35, 70, 70);

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
            p.setPen(CCanvas::penBorderBlue);
            p.setBrush(CCanvas::brushBackWhite);
            p.drawEllipse(pt2.u - 8, pt2.v - 8, 16, 16);
        }
    }

    // overlay points with the selected point icon
    foreach(i, selectedPoints)
    {
        XY pt = points[i];

        map.convertRad2Pt(pt.u, pt.v);
        p.drawPixmap(pt.u - 6, pt.v - 6, icon_BigRed);
    }

}




void COverlayDistance::calcDistance()
{
    distance = 0.0;

    if(points.count() < 2)
    {
        return;
    }

    double a1,a2;
    pt_t pt1, pt2;
    pt1 = points.first();
    points[0].idx = 0;

    for(int i = 1; i < points.count(); i++)
    {
        points[i].idx = i;
        pt2 = points[i];
        distance += ::distance(pt1, pt2, a1, a2);

        pt1 = pt2;
    }

}


void COverlayDistance::customMenu(QMenu& menu)
{
    menu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit..."),this,SLOT(slotEdit()));
    menu.addAction(QPixmap(":/icons/iconTrack16x16.png"),tr("Make Track"),this,SLOT(slotToTrack()));
    menu.addAction(QPixmap(":/icons/iconRoute16x16.png"),tr("Make Route"),this,SLOT(slotToRoute()));

}


void COverlayDistance::slotToTrack()
{
    if(points.isEmpty()) return;

    double dist, d, delta = 10.0, a1 , a2;
    XY pt1, pt2, ptx;
    CTrack::pt_t pt;
    CDlgConvertToTrack::EleMode_e eleMode;

    CDlgConvertToTrack dlg(0);
    if(dlg.exec() == QDialog::Rejected)
    {
        return;
    }

    CTrack * track  = new CTrack(&CTrackDB::self());
    track->setName(name);

    delta   = dlg.getDelta();
    eleMode = dlg.getEleMode();


    if(delta == -1)
    {

        for(int i = 0; i < points.count(); ++i)
        {
            pt2 = points[i];
            pt.lon = pt2.u * RAD_TO_DEG;
            pt.lat = pt2.v * RAD_TO_DEG;
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
                *track << pt;

                d += delta;
            }

            // and finally the next point
            pt.lon = pt2.u * RAD_TO_DEG;
            pt.lat = pt2.v * RAD_TO_DEG;
            *track << pt;

            pt1 = pt2;
        }
    }

    if(eleMode == CDlgConvertToTrack::eLocal)
    {
        track->replaceElevationByLocal();
    }
    else if(eleMode == CDlgConvertToTrack::eRemote)
    {
        track->replaceElevationByRemote();
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
    if(!overlayDistanceEditWidget.isNull()) delete overlayDistanceEditWidget;
    overlayDistanceEditWidget = new COverlayDistanceEditWidget(theMainWindow->getCanvas(), this);
    theMainWindow->setTempWidget(overlayDistanceEditWidget, tr("Overlay"));
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
    addType         = eNone;
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

void COverlayDistance::delPointsByIdx(const QList<int>& idx)
{
    int i;

    foreach(i, idx)
    {
        QList<pt_t>::iterator pt = points.begin();
        while(pt != points.end())
        {
            if(pt->idx == i)
            {
                pt = points.erase(pt);
            }
            else
            {
                pt++;
            }
        }
    }

    thePoint    = 0;
    doMove      = false;
    addType     = eNone;
    doFuncWheel = false;

    selectedPoints.clear();

    calcDistance();
    emit sigChanged();
}

