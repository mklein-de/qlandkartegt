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

#include "CMouseAddDistance.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "IMap.h"
#include "COverlayDB.h"
#include "COverlayDistance.h"
#include <QtGui>

CMouseAddDistance::CMouseAddDistance(CCanvas * canvas)
: IMouse(canvas)
{
    cursor = QCursor(QPixmap(":/cursors/cursorDistance.png"),0,0);
}


CMouseAddDistance::~CMouseAddDistance()
{

}


void CMouseAddDistance::mouseMoveEvent(QMouseEvent * e)
{
}


void CMouseAddDistance::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        canvas->setMouseMode(CCanvas::eMouseOverlay);

        pos      = e->pos();
        double x = e->pos().x();
        double y = e->pos().y();
        CMapDB::self().getMap().convertPt2Rad(x,y);
        COverlayDistance::pt_t pt;
        pt.u    = x;
        pt.v    = y;
        pt.idx  = 0;
        QList<COverlayDistance::pt_t> pts;
        pts << pt;
        selOverlay = COverlayDB::self().addDistance("", "", 0.0, pts);
    }
}


void CMouseAddDistance::mouseReleaseEvent(QMouseEvent * e)
{
}


