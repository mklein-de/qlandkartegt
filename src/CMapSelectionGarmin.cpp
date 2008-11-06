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
#include "CMapSelectionGarmin.h"
#include "CMapDB.h"
#include "IMap.h"

#include <QtGui>

CMapSelectionGarmin::CMapSelectionGarmin(QObject * parent)
: IMapSelection(eGarmin, parent)
{
    type = eGarmin;

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
    while(map != maps.end()){


        if(focusedMap == map.key()) {
            p.setPen(QPen(Qt::red,2));
        }
        else if(map.key() == theMap.getKey()) {
            p.setPen(QPen(Qt::darkBlue,2));
        }
        else {
            p.setPen(QPen(Qt::gray,2));
        }


        QMap<QString, tile_t>::const_iterator tile = map->tiles.begin();
        while(tile != map->tiles.end()){

            const QVector<double>& u = tile->u;
            const QVector<double>& v = tile->v;

            QPolygonF line;
            int N = u.size();
            for(n = 0; n < N; ++n){
                double x = u[n];
                double y = v[n];
                theMap.convertRad2Pt(x,y);
                line << QPointF(x,y);
            }

            if(rect.intersects(line.boundingRect().toRect())){
                p.drawPolygon(line);
            }

            ++tile;
        }
        ++map;
    }
}
