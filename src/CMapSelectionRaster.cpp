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
#include "CMapSelectionRaster.h"

#include <QtGui>
#include <projects.h>
#include "GeoMath.h"
#include "CMapDB.h"
#include "IMap.h"

CMapSelectionRaster::CMapSelectionRaster(QObject * parent)
: IMapSelection(eRaster, parent)
{
    type = eRaster;
}

CMapSelectionRaster::~CMapSelectionRaster()
{

}

void CMapSelectionRaster::draw(QPainter& p, const QRect& rect)
{
    IMap& map = CMapDB::self().getMap();

    double u1 = lon1;
    double v1 = lat1;
    double u2 = lon2;
    double v2 = lat2;

    map.convertRad2Pt(u1, v1);
    map.convertRad2Pt(u2, v2);

    p.setBrush(QColor(150,150,255,100));

    if(focusedMap == key) {
        p.setPen(QPen(Qt::red,2));
    }
    else if(mapkey == map.getKey()) {
        p.setPen(QPen(Qt::darkBlue,2));
    }
    else {
        p.setPen(QPen(Qt::gray,2));
    }

    QRect r(u1, v1, u2 - u1, v2 - v1);
    if(rect.intersects(r)) {
        p.drawRect(r);
        CCanvas::drawText(description,p,r);
    }
}

