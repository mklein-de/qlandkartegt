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
#include "CMouseAddWpt.h"
#include "CCanvas.h"
#include "CDlgEditWpt.h"
#include "CWptDB.h"
#include "CMapDB.h"

#include <QtGui>

CMouseAddWpt::CMouseAddWpt(CCanvas * canvas)
: IMouse(canvas)
{
    cursor = QCursor(QPixmap(":/cursors/cursorAdd"),0,0);
}


CMouseAddWpt::~CMouseAddWpt()
{

}


void CMouseAddWpt::mouseMoveEvent(QMouseEvent * e)
{

}


void CMouseAddWpt::mousePressEvent(QMouseEvent * e)
{
    IMap& map = CMapDB::self().getMap();

    canvas->setMouseMode(CCanvas::eMouseMoveArea);
    double u = e->pos().x();
    double v = e->pos().y();
    map.convertPt2Rad(u,v);
    float ele = map.getElevation(u,v);
    CWptDB::self().newWpt(u, v, ele);

}


void CMouseAddWpt::mouseReleaseEvent(QMouseEvent * e)
{
}
