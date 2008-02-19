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
#include "CMapRaster.h"
#include "CMainWindow.h"

#include "CMouseMoveMap.h"
#include "CMouseSelMap.h"
#include "CMouseAddWpt.h"

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
    setMouseMode(eMouseMoveArea);

}

CCanvas::~CCanvas()
{
}

void CCanvas::setMouseMode(mouse_mode_e mode)
{
    QApplication::restoreOverrideCursor();

    switch(mode){

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

        case eMouseSelectArea:
            mouse = mouseSelMap;
            break;

        default:;

    }
    if(underMouse()){
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

    draw(p);

    p.end();
}

void CCanvas::mouseMoveEvent(QMouseEvent * e)
{
    posMouse = e->pos();

    mouseMoveEventCoord(e);
    mouseMoveEventWpt(e);

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


void CCanvas::draw(QPainter& p)
{
    CMapDB::self().draw(p);
    mouse->draw(p);
    drawSearchResults(p);
    drawTracks(p);
    drawWaypoints(p);
}


void CCanvas::drawSearchResults(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();

    QMap<QString,CSearchDB::result_t>::const_iterator result = CSearchDB::self().begin();
    while(result != CSearchDB::self().end()){
        double u = result->lon * DEG_TO_RAD;
        double v = result->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(rect().contains(QPoint(u,v))){
            p.drawPixmap(u-16 , v-16, QPixmap(":/icons/iconBullseye16x16"));
        }

        ++result;
    }

}

void CCanvas::drawWaypoints(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();

    if(!selWpt.isNull()){
        double u = selWpt->lon * DEG_TO_RAD;
        double v = selWpt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        p.setPen(QColor(100,100,255,200));
        p.setBrush(QColor(255,255,255,200));
        p.drawEllipse(QRect(u - 11,  v - 11, 22, 22));

    }

    QMap<QString,CWpt*>::const_iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end()){
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(rect().contains(QPoint(u,v))){
            // draw waypoint icon
            p.drawPixmap(u-7 , v-7, getWptIconByName((*wpt)->icon));

            if((*wpt)->prx != WPT_NOFLOAT){
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
        }
        ++wpt;
    }
}

void CCanvas::drawTracks(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();
    QMap<QString,CTrack*> tracks                = CTrackDB::self().getTracks();
    QMap<QString,CTrack*>::iterator track       = tracks.begin();
    QMap<QString,CTrack*>::iterator highlighted = tracks.end();

    while(track != tracks.end()){
        QPolygon& line = (*track)->getPolyline();
        line.clear();

        QVector<CTrack::pt_t>& trkpts = (*track)->getTrackPoints();
        QVector<CTrack::pt_t>::iterator trkpt = trkpts.begin();
        while(trkpt != trkpts.end()) {
            double u = trkpt->lon * DEG_TO_RAD;
            double v = trkpt->lat * DEG_TO_RAD;

            map.convertRad2Pt(u,v);

            // skip deleted points, however if they are selected the
            // selection mark is shown
            if(trkpt->flags & CTrack::pt_t::eDeleted) {
//                 if(pt->flags & CGarminTrack::eSelected) {
//                     selected << pt->point;
//                 }
                ++trkpt; continue;
            }

            line << QPoint(u,v);

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
            p.setPen(QPen((*track)->getColor(),3));
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
        p.setPen(QPen((*track)->getColor(),5));
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
//         foreach(pt,selected) {
//             p.setPen(Qt::black);
//             p.setBrush(Qt::red);
//             p.drawEllipse(pt.x() - 3 ,pt.y() - 3,7,7);
//
//         }
    }
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

    switch(dir){

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
    QString info; // = QString("%1 %2, ").arg(e->x()).arg(e->y());

    double x = e->x();
    double y = e->y();
    map.convertPt2M(x,y);

    info += QString(" (%1 %2)").arg(x,0,'f',0).arg(y,0,'f',0);

    x = e->x();
    y = e->y();
    map.convertPt2Rad(x,y);

    float ele = map.getElevation(x,y);
    if(ele != WPT_NOFLOAT){
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

void CCanvas::mouseMoveEventWpt(QMouseEvent * e)
{
    IMap& map = CMapDB::self().getMap();
    CWpt * oldWpt = selWpt; selWpt = 0;

    QMap<QString,CWpt*>::const_iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end()){
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        QPoint diff = posMouse - QPoint(u,v);
        if(diff.manhattanLength() < 15){
            selWpt = *wpt;
            break;
        }

        ++wpt;
    }

    if(oldWpt != selWpt){
        if(selWpt){
            double u = selWpt->lon * DEG_TO_RAD;
            double v = selWpt->lat * DEG_TO_RAD;
            map.convertRad2Pt(u,v);

//             QFont f = CResources::self().getMapFont();
//             f.setBold(true);
//
//             info = new QLabel(selWpt->name,this);
//             info->setAutoFillBackground(true);
//             info->setFont(f);
//             info->move(u + 10 ,v - 20);
//             info->show();
        }
        else if(info){
            delete info;
            info = 0;
        }
        update();
    }
}

