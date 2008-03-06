/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
#include "CResources.h"
#include "CCanvas.h"
#include "CMapNoMap.h"
#include "CMapQMAP.h"
#include "CMainWindow.h"

#include "CMouseMoveMap.h"
#include "CMouseSelMap.h"
#include "CMouseAddWpt.h"
#include "CMouseMoveWpt.h"
#include "CMouseEditWpt.h"

#include "CWpt.h"
#include "CTrack.h"
#include "CSearchDB.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"

#include "GeoMath.h"
#include "WptIcons.h"

#include "CStatusCanvas.h"

#include <QtGui>

CCanvas::CCanvas(QWidget * parent)
: QWidget(parent)
, mouse(0)
, info(0)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    mouseMoveMap    = new CMouseMoveMap(this);
    mouseSelMap     = new CMouseSelMap(this);
    mouseAddWpt     = new CMouseAddWpt(this);
    mouseMoveWpt    = new CMouseMoveWpt(this);
    mouseEditWpt    = new CMouseEditWpt(this);
    setMouseMode(eMouseMoveArea);

}


CCanvas::~CCanvas()
{
}


void CCanvas::setMouseMode(mouse_mode_e mode)
{
    QApplication::restoreOverrideCursor();

    switch(mode) {

        case eMouseMoveArea:
            mouse = mouseMoveMap;
            break;

            //         case eMouseZoomArea:
            //             cursor = QCursor(cursorZoom,0,0);
            //             pfMousePressEvent   = &CCanvas::mousePressZoomArea;
            //             pfMouseMoveEvent    = &CCanvas::mouseMoveZoomArea;
            //             pfMouseReleaseEvent = &CCanvas::mouseReleaseZoomArea;
            //             break;
            //
        case eMouseAddWpt:
            mouse = mouseAddWpt;
            break;

        case eMouseEditWpt:
            mouse = mouseEditWpt;
            break;

        case eMouseMoveWpt:
            mouse = mouseMoveWpt;
            break;

        case eMouseSelectArea:
            mouse = mouseSelMap;
            break;

        default:;

    }
    if(underMouse()) {
        QApplication::setOverrideCursor(*mouse);
    }
    mouseMode = mode;
    update();
}


void CCanvas::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);
    emit sigResize(e->size());
}


void CCanvas::paintEvent(QPaintEvent * e)
{
    QWidget::paintEvent(e);

    QPainter p;
    p.begin(this);
    p.fillRect(rect(),Qt::white);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setFont(CResources::self().getMapFont());
    draw(p);

    p.end();
}


void CCanvas::mouseMoveEvent(QMouseEvent * e)
{
    posMouse = e->pos();

    mouseMoveEventCoord(e);
    mouse->mouseMoveEvent(e);
}


void CCanvas::mousePressEvent(QMouseEvent * e)
{
    mouse->mousePressEvent(e);
}


void CCanvas::mouseReleaseEvent(QMouseEvent * e)
{
    mouse->mouseReleaseEvent(e);
}


void CCanvas::enterEvent(QEvent * )
{
    QApplication::setOverrideCursor(*mouse);
}


void CCanvas::leaveEvent(QEvent * )
{
    QApplication::restoreOverrideCursor();
}


void CCanvas::print(QPrinter& printer)
{
    QPainter p;

    qreal s1 = (qreal)printer.pageRect().size().width() / (qreal)size().width();
    qreal s2 = (qreal)printer.pageRect().size().height() / (qreal)size().height();
    qreal s = (s1 > s2) ? s2 : s1;

    p.begin(&printer);
    p.scale(s,s);
    p.setClipRegion(rect());
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setFont(CResources::self().getMapFont());
    draw(p);
    p.end();

}


void CCanvas::draw(QPainter& p)
{
    CMapDB::self().draw(p);
    drawSearchResults(p);
    drawTracks(p);
    drawWaypoints(p);

    mouse->draw(p);
}


void CCanvas::drawSearchResults(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();

    QMap<QString,CSearchDB::result_t>::const_iterator result = CSearchDB::self().begin();
    while(result != CSearchDB::self().end()) {
        double u = result->lon * DEG_TO_RAD;
        double v = result->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(rect().contains(QPoint(u,v))) {
            p.drawPixmap(u-16 , v-16, QPixmap(":/icons/iconBullseye16x16"));
        }

        ++result;
    }
}


void CCanvas::drawWaypoints(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();

    QMap<QString,CWpt*>::const_iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end()) {
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(rect().contains(QPoint(u,v))) {
            QPixmap icon = getWptIconByName((*wpt)->icon);
            QPixmap back = QPixmap(icon.size());
            back.fill(Qt::white);
            back.setMask(icon.alphaChannel().createMaskFromColor(Qt::black));
            // draw waypoint icon
            p.drawPixmap(u-8 , v-8, back);
            p.drawPixmap(u-8 , v-7, back);
            p.drawPixmap(u-8 , v-6, back);
            p.drawPixmap(u-7 , v-8, back);

            p.drawPixmap(u-7 , v-6, back);
            p.drawPixmap(u-6 , v-8, back);
            p.drawPixmap(u-6 , v-7, back);
            p.drawPixmap(u-6 , v-6, back);

            p.drawPixmap(u-7 , v-7, icon);

            if((*wpt)->prx != WPT_NOFLOAT) {
                double u1 = u;
                double v1 = v;
                map.convertPt2M(u1,v1);
                double u2 = u1 + (*wpt)->prx;
                double v2 = v1;
                map.convertM2Pt(u2,v2);
                double r = u2 - u;

                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(Qt::white,3));
                p.drawEllipse(QRect(u - r - 1, v - r - 1, 2*r + 1, 2*r + 1));
                p.setPen(QPen(Qt::red,1));
                p.drawEllipse(QRect(u - r - 1, v - r - 1, 2*r + 1, 2*r + 1));
            }

            drawText((*wpt)->name,p,QPoint(u,v - 10));

        }
        ++wpt;
    }
}


void CCanvas::drawTracks(QPainter& p)
{
    QPoint focus(-1,-1);
    QVector<QPoint> selected;
    IMap& map = CMapDB::self().getMap();
    QMap<QString,CTrack*> tracks                = CTrackDB::self().getTracks();
    QMap<QString,CTrack*>::iterator track       = tracks.begin();
    QMap<QString,CTrack*>::iterator highlighted = tracks.end();

    while(track != tracks.end()) {
        QPolygon& line = (*track)->getPolyline();
        line.clear();

        QVector<CTrack::pt_t>& trkpts = (*track)->getTrackPoints();
        QVector<CTrack::pt_t>::iterator trkpt = trkpts.begin();
        while(trkpt != trkpts.end()) {
            double u = trkpt->lon * DEG_TO_RAD;
            double v = trkpt->lat * DEG_TO_RAD;

            map.convertRad2Pt(u,v);
            trkpt->px = QPoint(u,v);

            if((*track)->isHighlighted() && trkpt->flags & CTrack::pt_t::eSelected) {
                selected << trkpt->px;
            }

            if((*track)->isHighlighted() && trkpt->flags & CTrack::pt_t::eFocus) {
                focus = trkpt->px;
            }

            // skip deleted points, however if they are selected the
            // selection mark is shown
            if(trkpt->flags & CTrack::pt_t::eDeleted) {
                ++trkpt; continue;
            }

            line << trkpt->px;
            ++trkpt;
        }

        if(!rect().intersects(line.boundingRect())) {
            ++track; continue;
        }

        if((*track)->isHighlighted()) {
            // store highlighted track to draw it later
            // it must be drawn above all other tracks
            highlighted = track;
        }
        else {
            // draw normal track
            QPen pen((*track)->getColor(),3);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            p.setPen(pen);
            p.drawPolyline(line);
            p.setPen(Qt::white);
            p.drawPolyline(line);
        }

        ++track;
    }

    // if there is a highlighted track, draw it
    if(highlighted != tracks.end()) {
        track = highlighted;

        QPolygon& line = (*track)->getPolyline();

        // draw skunk line
        QPen pen((*track)->getColor(),5);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        p.drawPolyline(line);
        p.setPen(Qt::white);
        p.drawPolyline(line);

        // draw bubbles
        QPoint pt;
        foreach(pt,line) {
            p.setPen((*track)->getColor());
            p.setBrush(Qt::white);
            p.drawEllipse(pt.x() - 2 ,pt.y() - 2,5,5);

        }
        foreach(pt,selected) {
            p.setPen(Qt::black);
            p.setBrush(Qt::red);
            p.drawEllipse(pt.x() - 3 ,pt.y() - 3,7,7);

        }

        if(focus != QPoint(-1,-1)) {
            p.setPen(QPen(Qt::white,3));
            p.drawLine(focus + QPoint(0,-20),focus + QPoint(0,20));
            p.drawLine(focus + QPoint(-20,0),focus + QPoint(20,0));
            p.setPen(Qt::red);
            p.drawLine(focus + QPoint(0,-20),focus + QPoint(0,20));
            p.drawLine(focus + QPoint(-20,0),focus + QPoint(20,0));
        }
    }
}


void CCanvas::drawText(const QString& str, QPainter& p, const QPoint& center)
{
    QFont           f = CResources::self().getMapFont();
    QFontMetrics    fm(f);
    QRect           r = fm.boundingRect(str);

    r.moveCenter(center);

    p.setPen(Qt::white);
    p.setFont(f);

    p.drawText(r.topLeft() - QPoint(-1,-1), str);
    p.drawText(r.topLeft() - QPoint(-1,+1), str);
    p.drawText(r.topLeft() - QPoint(+1,-1), str);
    p.drawText(r.topLeft() - QPoint(+1,+1), str);

    p.setFont(f);
    p.setPen(Qt::darkBlue);
    p.drawText(r.topLeft(),str);

}


void CCanvas::wheelEvent(QWheelEvent * e)
{
    zoom((e->delta() < 0), e->pos());
}


void CCanvas::zoom(bool in, const QPoint& p)
{
    CMapDB::self().getMap().zoom(in, p);
    update();
}


void CCanvas::move(double lon, double lat)
{
    IMap& map = CMapDB::self().getMap();
    double u = lon * DEG_TO_RAD;
    double v = lat * DEG_TO_RAD;
    map.convertRad2Pt(u,v);
    map.move(QPoint(u,v), rect().center());
    update();
}


void CCanvas::move(move_direction_e dir)
{
    IMap& map = CMapDB::self().getMap();
    QPoint p1 = geometry().center();
    QPoint p2 = p1;

    switch(dir) {

        case eMoveLeft:
            p2.rx() += width() / 4;
            break;

        case eMoveRight:
            p2.rx() -= width() / 4;
            break;

        case eMoveUp:
            p2.ry() += height() / 4;
            break;

        case eMoveDown:
            p2.ry() -= height() / 4;
            break;

        case eMoveCenter:
        {
            double lon1 = 0, lat1 = 0, lon2 = 0, lat2 = 0;

            map.dimensions(lon1, lat1, lon2, lat2);
            lon1 += (lon2 - lon1)/2;
            lat2 += (lat1 - lat2)/2;
            map.convertRad2Pt(lon1,lat2);

            p1.rx() = lon1;
            p1.ry() = lat2;

            p2 = geometry().center();
        }
        break;
    }
    map.move(p1, p2);

    update();
}


void CCanvas::mouseMoveEventCoord(QMouseEvent * e)
{
    IMap& map = CMapDB::self().getMap();
    QString info;                // = QString("%1 %2, ").arg(e->x()).arg(e->y());

    double x = e->x();
    double y = e->y();
    map.convertPt2M(x,y);

    info += QString(" (%1 %2)").arg(x,0,'f',0).arg(y,0,'f',0);

    x = e->x();
    y = e->y();
    map.convertPt2Rad(x,y);

    float ele = map.getElevation(x,y);
    if(ele != WPT_NOFLOAT) {
        info += QString(" (ele: %1 m)").arg(ele);
    }

    x *= RAD_TO_DEG;
    y *= RAD_TO_DEG;

    qint32 degN,degE;
    float minN,minE;

    GPS_Math_Deg_To_DegMin(y, &degN, &minN);

    GPS_Math_Deg_To_DegMin(x, &degE, &minE);

    QString str,lat,lng;
    lat = degN < 0 ? "S" : "N";
    lng = degE < 0 ? "W" : "E";
    str.sprintf(" %s%02d\260 %06.3f %s%03d\260 %06.3f ",lat.toUtf8().data(),abs(degN),minN,lng.toUtf8().data(),abs(degE),minE);

    info += str;

    theMainWindow->setPositionInfo(info);

}
