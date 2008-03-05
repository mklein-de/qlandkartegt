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

#include "CMouseMoveMap.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "CWptDB.h"
#include "CTrackDB.h"

#include <QtGui>

CMouseMoveMap::CMouseMoveMap(CCanvas * parent)
: IMouse(parent)
, moveMap(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorMoveMap"),0,0);
}


CMouseMoveMap::~CMouseMoveMap()
{

}


void CMouseMoveMap::mouseMoveEvent(QMouseEvent * e)
{
    if(moveMap) {
        CMapDB::self().getMap().move(oldPoint, e->pos());
        oldPoint = e->pos();
        canvas->update();
    }

    mouseMoveEventWpt(e);
    mouseMoveEventTrack(e);
}


void CMouseMoveMap::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        cursor = QCursor(QPixmap(":/cursors/cursorMove"));
        QApplication::setOverrideCursor(cursor);
        moveMap     = true;
        oldPoint    = e->pos();

        if(selWpt) {
            CWptDB::self().selWptByKey(selWpt->key());
        }

        CTrack * track = CTrackDB::self().highlightedTrack();
        if(track && selTrkPt) {
            track->setPointOfFocus(selTrkPt->idx);
        }
    }
}


void CMouseMoveMap::mouseReleaseEvent(QMouseEvent * e)
{
    if(moveMap && (e->button() == Qt::LeftButton)) {
        moveMap = false;
        cursor = QCursor(QPixmap(":/cursors/cursorMoveMap"),0,0);
        QApplication::restoreOverrideCursor();
        canvas->update();
    }
}


void CMouseMoveMap::draw(QPainter& p)
{
    drawSelWpt(p);
    drawSelTrkPt(p);
}
