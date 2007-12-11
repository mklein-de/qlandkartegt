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

#include "IMouse.h"
#include "CCanvas.h"
#include <QtGui>

IMouse::IMouse(CCanvas * canvas)
    : QObject(canvas)
    , cursor(QPixmap(":/cursor/Arrow"))
    , canvas(canvas)
{

}

IMouse::~IMouse()
{

}

void IMouse::startRect(const QPoint& p)
{
    rect.setTopLeft(p);
    rect.setSize(QSize(0,0));
}

void IMouse::resizeRect(const QPoint& p)
{
    rect.setBottomRight(p);
    canvas->update();
}

void IMouse::drawRect(QPainter& p)
{
    p.setBrush(QBrush( QColor(230,230,255,100) ));
    p.setPen(QColor(150,150,255));
    p.drawRect(rect);
}
