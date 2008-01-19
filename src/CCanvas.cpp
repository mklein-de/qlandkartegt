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

#include "CCanvas.h"
#include "CMapNoMap.h"
#include "CMapRaster.h"
#include "CMainWindow.h"

#include "CMouseMoveMap.h"
#include "CMouseSelMap.h"
#include "CMouseAddWpt.h"

#include "CSearchDB.h"

#include <QtGui>


static void GPS_Math_Deg_To_DegMin(double v, int32_t *d, double *m)
{
    int32_t sign;

    if(v<(double)0.)
    {
        v *= (double)-1.;
        sign = 1;
    }
    else{
        sign = 0;
    }

    *d = (int32_t)v;
    *m = (v-(double)*d) * (double)60.0;
    if(*m>(double)59.999)
    {
        ++*d;
        *m = (double)0.0;
    }

    if(sign){
        *d = -*d;
    }

    return;
}


CCanvas::CCanvas(QWidget * parent)
    : QWidget(parent)
    , mouse(0)
    , moveMap(false)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    map = new CMapNoMap(this);


    mouseMoveMap = new CMouseMoveMap(this);
    mouseSelMap = new CMouseSelMap(this);
    mouseAddWpt = new CMouseAddWpt(this);
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
    map->resize(e->size());
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


void CCanvas::draw(QPainter& p)
{
    map->draw(p);
    mouse->draw(p);
    drawSearchResults(p);
}

void CCanvas::drawSearchResults(QPainter& p)
{
    QMap<QString,CSearchDB::result_t>::const_iterator result = CSearchDB::self().begin();
    while(result != CSearchDB::self().end()){
        double u = result->lon * DEG_TO_RAD;
        double v = result->lat * DEG_TO_RAD;
        map->convertRad2Pt(u,v);

        if(rect().contains(QPoint(u,v))){
            p.drawPixmap(u-8 , v-8, QPixmap(":/icons/iconBullseye16x16"));
        }

        ++result;
    }

}


void CCanvas::wheelEvent(QWheelEvent * e)
{
  zoom((e->delta() < 0), e->pos());
}

void CCanvas::loadMapSet(const QString& filename)
{
    delete map;
    map = new CMapRaster(filename,this);
    qDebug() << map;
    map->resize(size());
}

void CCanvas::zoom(bool in, const QPoint& p)
{
    map->zoom(in, p);
    update();
}

void CCanvas::move(double lon, double lat)
{
    double u = lon * DEG_TO_RAD;
    double v = lat * DEG_TO_RAD;
    map->convertRad2Pt(u,v);
    map->move(QPoint(u,v), rect().center());
    update();
}

void CCanvas::move(move_direction_e dir)
{
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
    }
    map->move(p1, p2);

    update();
}


void CCanvas::mouseMoveEventCoord(QMouseEvent * e)
{
    QString info = QString("%1 %2, ").arg(e->x()).arg(e->y());

    double x = e->x();
    double y = e->y();
    map->convertPt2M(x,y);

    info += QString(" (%1 %2)").arg(x,0,'f',0).arg(y,0,'f',0);

    x = e->x();
    y = e->y();
    map->convertPt2Rad(x,y);
    x *= RAD_TO_DEG;
    y *= RAD_TO_DEG;

    qint32 degN,degE;
    double minN,minE;

    GPS_Math_Deg_To_DegMin(y, &degN, &minN);

    GPS_Math_Deg_To_DegMin(x, &degE, &minE);

    QString str,lat,lng;
    lat = degN < 0 ? "S" : "N";
    lng = degE < 0 ? "W" : "E";
    str.sprintf(" %s%02d\260 %06.3f %s%03d\260 %06.3f ",lat.toUtf8().data(),abs(degN),minN,lng.toUtf8().data(),abs(degE),minE);

    info += str;

    theMainWindow->setPositionInfo(info);

}
