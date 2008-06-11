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

#include "COverlayText.h"

#include <QtGui>

COverlayText::COverlayText(const QRect& rect, QObject * parent)
: IOverlay(parent, "Text", QPixmap(":/icons/iconText16x16"))
, rect(rect)
{

}

COverlayText::~COverlayText()
{

}

void COverlayText::draw(QPainter& p)
{
    p.setBrush(Qt::white);
    p.setPen(Qt::black);
    p.drawRect(rect);

    if(selected == this){
        p.setBrush(Qt::white);
        p.setPen(QPen(Qt::red, 2));
        p.drawRect(rect);

        p.drawPixmap(rect.topLeft() + QPoint(2,2), QPixmap(":/icons/iconMoveMap16x16.png"));
        p.drawPixmap(rect.bottomRight() - QPoint(16,16), QPixmap(":/icons/iconSize16x16.png"));
    }
}

