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
#include "CMapSelectionGarmin.h"
#include "CMapDB.h"
#include "IMap.h"

#include <QtGui>
#include <projects.h>
#ifdef __MINGW32__
#undef LP
#endif

CMapSelectionGarmin::CMapSelectionGarmin(QObject * parent)
: IMapSelection(eGarmin, parent)
, tilecnt(0)
{
}


CMapSelectionGarmin::~CMapSelectionGarmin()
{

}


void CMapSelectionGarmin::draw(QPainter& p, const QRect& rect)
{
    int n;
    IMap& theMap = CMapDB::self().getMap();

    p.setBrush(QColor(150,150,255,100));

    QMap<QString,map_t>::const_iterator map = maps.begin();
    while(map != maps.end())
    {

        if(focusedMap == "gmapsupp")
        {
            p.setPen(QPen(Qt::red,2));
        }
        else if(map.key() == theMap.getKey())
        {
            p.setPen(QPen(Qt::darkBlue,2));
        }
        else
        {
            p.setPen(QPen(Qt::gray,2));
        }

        QMap<QString, tile_t>::const_iterator tile = map->tiles.begin();
        while(tile != map->tiles.end())
        {

            const QVector<double>& u = tile->u;
            const QVector<double>& v = tile->v;

            QPolygonF line;
            int N = u.size();
            for(n = 0; n < N; ++n)
            {
                double x = u[n];
                double y = v[n];
                theMap.convertRad2Pt(x,y);
                line << QPointF(x,y);
            }

            if(rect.intersects(line.boundingRect().toRect()))
            {
                p.drawPolygon(line);
            }

            ++tile;
        }
        ++map;
    }
}


quint32 CMapSelectionGarmin::getMemSize()
{
    quint32 memSize = 0;

    QMap<QString,map_t>::const_iterator map = maps.begin();
    while(map != maps.end())
    {

        QMap<QString, tile_t>::const_iterator tile = map->tiles.begin();
        while(tile != map->tiles.end())
        {
            memSize += tile->memSize;
            ++tile;
        }
        ++map;
    }
    return memSize;
}


void CMapSelectionGarmin::calcArea()
{
    tilecnt = 0;

    lat1 =  -90.0 * DEG_TO_RAD;
    lon2 = -180.0 * DEG_TO_RAD;
    lat2 =   90.0 * DEG_TO_RAD;
    lon1 =  180.0 * DEG_TO_RAD;

    QMap<QString,map_t>::const_iterator map = maps.begin();
    while(map != maps.end())
    {

        QMap<QString, tile_t>::const_iterator tile = map->tiles.begin();
        while(tile != map->tiles.end())
        {
            const QRectF& r = tile->area;
            if(lat1 < r.top())      lat1 = r.top();
            if(lon2 < r.right())    lon2 = r.right();
            if(lat2 > r.bottom())   lat2 = r.bottom();
            if(lon1 > r.left())     lon1 = r.left();

            ++tile; ++tilecnt;
        }
        ++map;
    }
}
