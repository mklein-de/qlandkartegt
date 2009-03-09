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

#include "CMouseZoomMap.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "IMap.h"

#include <QtGui>

CMouseZoomMap::CMouseZoomMap(CCanvas * parent)
: IMouse(parent)
, zoomMap(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorZoom"),0,0);
}


CMouseZoomMap::~CMouseZoomMap()
{

}


void CMouseZoomMap::mouseMoveEvent(QMouseEvent * e)
{
    if(!zoomMap) return;
    resizeRect(e->pos());

}


void CMouseZoomMap::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        startRect(e->pos());
        zoomMap = true;
    }
}


void CMouseZoomMap::mouseReleaseEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        zoomMap = false;
        resizeRect(e->pos());

        rect = rect.normalized();

        if(rect.width() < 10) {
            rect.setWidth(10);
        }
        if(rect.height() < 10) {
            rect.setHeight(10);
        }

        double lon1 = rect.left();
        double lat1 = rect.top();
        double lon2 = rect.right();
        double lat2 = rect.bottom();

        IMap& map = CMapDB::self().getMap();
        map.convertPt2Rad(lon1,lat1);
        map.convertPt2Rad(lon2,lat2);
        map.zoom(lon1, lat1, lon2, lat2);

        canvas->setMouseMode(CCanvas::eMouseMoveArea);
    }
}


void CMouseZoomMap::draw(QPainter& p)
{
    if(!zoomMap) return;
    drawRect(p);
}
