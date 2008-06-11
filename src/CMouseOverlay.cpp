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

#include "CMouseOverlay.h"

CMouseOverlay::CMouseOverlay(CCanvas * canvas)
: IMouse(canvas)
{
    cursor = QCursor(QPixmap(":/cursors/cursorArrow"),0,0);
}

CMouseOverlay::~CMouseOverlay()
{

}

void CMouseOverlay::mouseMoveEvent(QMouseEvent * e)
{
    mouseMoveEventOverlay(e);
}

void CMouseOverlay::mousePressEvent(QMouseEvent * e)
{
}

void CMouseOverlay::mouseReleaseEvent(QMouseEvent * e)
{
}


