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
#ifdef __MINGW32__
#undef LP
#endif
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

    if(focusedMap == key)
    {
        p.setPen(QPen(Qt::red,2));
    }
    else if(mapkey == map.getKey())
    {
        p.setPen(QPen(Qt::darkBlue,2));
    }
    else
    {
        p.setPen(QPen(Qt::gray,2));
    }

    QRect r(u1, v1, u2 - u1, v2 - v1);
    if(rect.intersects(r))
    {
        p.drawRect(r);

        int i;
        quint32 gridspace = map.scalePixelGrid(1024);

        for(i = r.left(); i < r.right(); i+= gridspace)
        {
            p.drawLine(i, r.top(), i, r.bottom());
        }

        for(i = r.top(); i < r.bottom(); i+= gridspace)
        {
            p.drawLine(r.left(), i, r.right(), i);
        }

        CCanvas::drawText(getDescription(),p,r);
    }
}

QString CMapSelectionRaster::getDescription()
{
    QString pos1, pos2, str;
    GPS_Math_Deg_To_Str(lon1 * RAD_TO_DEG, lat1 * RAD_TO_DEG, pos1);
    GPS_Math_Deg_To_Str(lon2 * RAD_TO_DEG, lat2 * RAD_TO_DEG, pos2);

    double a1, a2, d1, d2;
    XY p1, p2;

    p1.u = lon1;
    p1.v = lat1;

    p2.u = lon2;
    p2.v = lat1;

    d1 = distance(p1, p2, a1, a2) / 1000.0;


    p1.u = lon1;
    p1.v = lat1;

    p2.u = lon1;
    p2.v = lat2;

    d2 = distance(p1, p2, a1, a2) / 1000.0;

    str  = description + "\n" + pos1 + "\n" + pos2;
    str += "\n" + QString("%1 x %2 km = %3 km%4").arg(d1,0,'f',1).arg(d2,0,'f',1).arg(d1*d2,0,'f',1).arg(QChar(0x00B2));

    return str;

}

