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

#include "CMouseAddDistance.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "IMap.h"
#include "COverlayDB.h"
#include "COverlayDistance.h"
#include <QtGui>

CMouseAddDistance::CMouseAddDistance(CCanvas * canvas)
: IMouse(canvas)
{
    cursor = QCursor(QPixmap(":/cursors/cursorDistance"),0,0);
}


CMouseAddDistance::~CMouseAddDistance()
{

}


void CMouseAddDistance::mouseMoveEvent(QMouseEvent * e)
{
    if(overlay.isNull()) return;
    pos = e->pos();
    canvas->update();
}


void CMouseAddDistance::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton && overlay.isNull()) {
        pos      = e->pos();
        double x = e->pos().x();
        double y = e->pos().y();
        CMapDB::self().getMap().convertPt2Rad(x,y);
        XY pt;
        pt.u = x;
        pt.v = y;
        QList<XY> pts;
        pts << pt;
        overlay = COverlayDB::self().addDistance("", "", pts);
    }
    else if(e->button() == Qt::LeftButton && !overlay.isNull()) {
        double x = e->pos().x();
        double y = e->pos().y();
        CMapDB::self().getMap().convertPt2Rad(x,y);
        XY pt;
        pt.u = x;
        pt.v = y;

        overlay->addPoint(pt);
    }
    else if(e->button() == Qt::RightButton) {
        overlay = 0;
        canvas->setMouseMode(CCanvas::eMouseMoveArea);
    }
}


void CMouseAddDistance::mouseReleaseEvent(QMouseEvent * e)
{
}


void CMouseAddDistance::draw(QPainter& p)
{
    if(overlay.isNull()) return;

    XY pt1 = overlay->getLast();
    CMapDB::self().getMap().convertRad2Pt(pt1.u, pt1.v);

    p.setPen(QPen(Qt::white, 3));
    p.drawLine(pt1.u, pt1.v, pos.x(), pos.y());
    p.setPen(QPen(Qt::red, 1));
    p.drawLine(pt1.u, pt1.v, pos.x(), pos.y());
}
