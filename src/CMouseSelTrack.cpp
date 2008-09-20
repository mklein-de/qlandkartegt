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

#include "CMouseSelTrack.h"
#include "CCanvas.h"
#include "CTrackDB.h"

#include <QtGui>

CMouseSelTrack::CMouseSelTrack(CCanvas * canvas)
: IMouse(canvas)
, selTrack(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorArrow"),0,0);
}

CMouseSelTrack::~CMouseSelTrack()
{

}

void CMouseSelTrack::mouseMoveEvent(QMouseEvent * e)
{
    if(!selTrack) return;
    resizeRect(e->pos());
}

void CMouseSelTrack::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        startRect(e->pos());
        selTrack = true;
    }
}

void CMouseSelTrack::mouseReleaseEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        selTrack = false;
        resizeRect(e->pos());
        CTrackDB::self().select(rect.normalized());
//         canvas->setMouseMode(CCanvas::eMouseMoveArea);
    }
}

void CMouseSelTrack::draw(QPainter& p)
{
    if(!selTrack) return;
    drawRect(p);
}

