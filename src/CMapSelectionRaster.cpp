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

    p.setBrush(Qt::NoBrush);

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

        quint32 gridspace = map.scalePixelGrid(1024);


        if(gridspace != 0)
        {
            int pxx,pxy, x, y;
            for(pxx = r.left(), x = 0; pxx < r.right(); pxx += gridspace, x++)
            {
                for(pxy = r.top(), y = 0; pxy < r.bottom(); pxy += gridspace, y++)
                {
                    int w = (r.right() - pxx) > gridspace ? gridspace : (r.right() - pxx);
                    int h = (r.bottom() - pxy) > gridspace ? gridspace : (r.bottom() - pxy);
                    QRect rect(pxx,pxy, w, h);

                    QPair<int,int> index(x,y);

                    if(!selTiles.contains(index))
                    {
                        selTiles[index] = false;
                    }

                    if(selTiles[index])
                    {
                        p.setBrush(Qt::NoBrush);
                    }
                    else
                    {
                        p.setBrush(QColor(150,150,255,100));
                    }

                    p.drawRect(rect);

                }
            }
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

    int tileCount = 0, i;
    foreach(i, selTiles)
    {
        tileCount += i ? 0 : 1;
    }

    str  = description;
    str += "\n" + tr("Tiles: #%1").arg(tileCount);
    return str;

}

