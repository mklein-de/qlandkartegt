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

#include "CMouseMoveWpt.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "WptIcons.h"
#include "CMapDB.h"
#include "GeoMath.h"
#include "CResources.h"

#include <QtGui>

CMouseMoveWpt::CMouseMoveWpt(CCanvas * canvas)
: IMouse(canvas)
, moveWpt(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorMoveWpt"),0,0);
}


CMouseMoveWpt::~CMouseMoveWpt()
{

}


void CMouseMoveWpt::mouseMoveEvent(QMouseEvent * e)
{
    if(moveWpt) {
        newPos = e->pos();
        theMainWindow->getCanvas()->update();
    }
    else {
        mouseMoveEventWpt(e);
    }
}


void CMouseMoveWpt::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        if(!moveWpt && !selWpt.isNull()) {
            newPos = e->pos();
            moveWpt = true;
        }
        else if(moveWpt && !selWpt.isNull()) {
            IMap& map = CMapDB::self().getMap();
            double u = e->pos().x();
            double v = e->pos().y();
            map.convertPt2Rad(u,v);
            selWpt->lon = u * RAD_TO_DEG;
            selWpt->lat = v * RAD_TO_DEG;
            moveWpt = false;
            canvas->setMouseMode(CCanvas::eMouseMoveArea);
        }
    }
    else if(e->button() == Qt::RightButton) {
        moveWpt = false;
        canvas->setMouseMode(CCanvas::eMouseMoveArea);
    }
    theMainWindow->getCanvas()->update();
}


void CMouseMoveWpt::mouseReleaseEvent(QMouseEvent * e)
{
}


void CMouseMoveWpt::draw(QPainter& p)
{
    if(moveWpt && !selWpt.isNull()) {
        double x1, y1, x2, y2;
        XY p1, p2;
        IMap& map = CMapDB::self().getMap();

        x1 = p1.u = selWpt->lon * DEG_TO_RAD;
        y1 = p1.v = selWpt->lat * DEG_TO_RAD;
        map.convertRad2Pt(x1,y1);

        x2 = p2.u = newPos.x();
        y2 = p2.v = newPos.y();
        map.convertPt2Rad(p2.u,p2.v);

        double a1 = 0, a2 = 0, d;
        d = distance(p1,p2,a1,a2);

        QString str;

        if(CResources::self().doMetric()) {
            if(d > 9999) {
                str = QString(" %1km %2\260> <%3\260 ").arg(d/1000,0,'f',2).arg(a1,0,'f',0).arg(a2,0,'f',0);
            }
            else {
                str = QString(" %1m %2\260> <%3\260 ").arg(d,0,'f',0).arg(a1,0,'f',0).arg(a2,0,'f',0);
            }
        }
        else {
            str = QString(" %1ml %2\260> <%3\260 ").arg(d * 0.6213699E-3,0,'f',2).arg(a1,0,'f',0).arg(a2,0,'f',0);
        }

        // draw line between old and new location
        p.setPen(QPen(Qt::white,3));
        p.drawLine(x1, y1, x2, y2);
        p.setPen(Qt::darkBlue);
        p.drawLine(x1, y1, x2, y2);

        // draw waypoint icon
        QPixmap icon = getWptIconByName(selWpt->icon);
        QPixmap back = QPixmap(icon.size());
        back.fill(Qt::white);
        back.setMask(icon.alphaChannel().createMaskFromColor(Qt::black));

        p.drawPixmap(x2 - 8 , y2 - 8, back);
        p.drawPixmap(x2 - 8 , y2 - 7, back);
        p.drawPixmap(x2 - 8 , y2 - 6, back);
        p.drawPixmap(x2 - 7 , y2 - 8, back);

        p.drawPixmap(x2 - 7 , y2 - 6, back);
        p.drawPixmap(x2 - 6 , y2 - 8, back);
        p.drawPixmap(x2 - 6 , y2 - 7, back);
        p.drawPixmap(x2 - 6 , y2 - 6, back);

        p.drawPixmap(x2 - 7 , y2 - 7, icon);

        CCanvas::drawText(str, p, QPoint(x2,y2 - 5));
    }
    else {
        drawSelWpt(p);
    }
}
