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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CMouseSelMap.h"
#include "CCanvas.h"
#include "CMapDB.h"

#include <QtGui>


CMouseSelMap::CMouseSelMap(CCanvas * canvas)
: IMouse(canvas)
, selMap(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorSelMap.png"),0,0);
}


CMouseSelMap::~CMouseSelMap()
{

}


void CMouseSelMap::draw(QPainter& p)
{
    if(!selMap) return;

    IMap& map = CMapDB::self().getMap();
    quint32 gridspace = map.scalePixelGrid(TILESIZE);

    if(gridspace != 0)
    {
        // snap grid if parts are too small
        int w = rect.width() % gridspace;
        int h = rect.height() % gridspace;

        if(w < (gridspace * 0.20))
        {
            w = - w;
        }
        else if(w > (gridspace * 0.80))
        {
            w = gridspace - w;
        }
        else
        {
            w = 0;
        }

        if(h < (gridspace * 0.20))
        {
            h = - h;
        }
        else if(h > (gridspace * 0.80))
        {
            h = gridspace - h;
        }
        else
        {
            h = 0;
        }

        rect.adjust(0,0,w,h);
    }

    if(gridspace == 0)
    {
        return;
    }

    selTiles.clear();
    int pxx,pxy, x, y;

    p.setBrush(QColor(150,150,255,100));
    p.setPen(QPen(Qt::darkBlue,2));

    for(pxx = rect.left(), x = 0; pxx < rect.right(); pxx += gridspace, x++)
    {
        for(pxy = rect.top(), y = 0; pxy < rect.bottom(); pxy += gridspace, y++)
        {
            int w = (rect.right() - pxx) > gridspace ? gridspace : (rect.right() - pxx);
            int h = (rect.bottom() - pxy) > gridspace ? gridspace : (rect.bottom() - pxy);
            QRect r(pxx,pxy, w, h);

            QPair<int,int> index(x,y);
            selTiles[index] = false;

            p.drawRect(r);
        }
    }


}


void CMouseSelMap::mouseMoveEvent(QMouseEvent * e)
{
    if(!selMap) return;
    resizeRect(e->pos());
}


void CMouseSelMap::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        startRect(e->pos());
        selMap = true;
    }
}


void CMouseSelMap::mouseReleaseEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        selMap = false;
        resizeRect(e->pos());

        rect = rect.normalized();

        if(rect.width() < 2)
        {
            rect.setWidth(2);
        }
        if(rect.height() < 2)
        {
            rect.setHeight(2);
        }


        // snap grid if parts are too small
        IMap& map = CMapDB::self().getMap();
        quint32 gridspace = map.scalePixelGrid(TILESIZE);

        if(gridspace != 0)
        {
            int w = rect.width() % gridspace;
            int h = rect.height() % gridspace;

            if(w < (gridspace * 0.20))
            {
                w = - w;
            }
            else if(w > (gridspace * 0.80))
            {
                w = gridspace - w;
            }
            else
            {
                w = 0;
            }

            if(h < (gridspace * 0.20))
            {
                h = - h;
            }
            else if(h > (gridspace * 0.80))
            {
                h = gridspace - h;
            }
            else
            {
                h = 0;
            }
            rect.adjust(0,0,w,h);
        }


        CMapDB::self().select(rect, selTiles);
        canvas->setMouseMode(CCanvas::eMouseMoveArea);
    }
}
