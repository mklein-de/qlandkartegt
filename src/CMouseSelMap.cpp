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

#include "CMouseSelMap.h"
#include "CCanvas.h"
#include "CMapDB.h"

#include <QtGui>

CMouseSelMap::CMouseSelMap(CCanvas * canvas)
: IMouse(canvas)
, selMap(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorSelMap"),0,0);
}


CMouseSelMap::~CMouseSelMap()
{

}


void CMouseSelMap::draw(QPainter& p)
{
    if(!selMap) return;
    drawRect(p);
}


void CMouseSelMap::mouseMoveEvent(QMouseEvent * e)
{
    if(!selMap) return;
    resizeRect(e->pos());
}


void CMouseSelMap::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        startRect(e->pos());
        selMap = true;
    }
}


void CMouseSelMap::mouseReleaseEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        selMap = false;
        resizeRect(e->pos());

        rect = rect.normalized();

        if(rect.width() < 2){
            rect.setWidth(2);
        }
        if(rect.height() < 2){
            rect.setHeight(2);
        }

        CMapDB::self().select(rect);
        canvas->setMouseMode(CCanvas::eMouseMoveArea);
    }
}
