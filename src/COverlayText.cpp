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
#include "CMainWindow.h"
#include "CCanvas.h"

#include <QtGui>

COverlayText::COverlayText(const QRect& rect, QObject * parent)
: IOverlay(parent, "Text", QPixmap(":/icons/iconText16x16"))
, rect(rect)
, doMove(false)
, doSize(false)
{
    rectMove = QRect(rect.topLeft()     + QPoint(2,2)  , QSize(16, 16));
    rectSize = QRect(rect.bottomRight() - QPoint(16,16), QSize(16, 16));
    rectEdit = QRect(rect.topRight()    - QPoint(36,-2), QSize(16, 16));
    rectDel  = QRect(rect.topRight()    - QPoint(18,-2), QSize(16, 16));
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

        p.drawPixmap(rectMove, QPixmap(":/icons/iconMoveMap16x16.png"));
        p.drawPixmap(rectSize, QPixmap(":/icons/iconSize16x16.png"));
        p.drawPixmap(rectDel, QPixmap(":/icons/iconClear16x16.png"));
        p.drawPixmap(rectEdit, QPixmap(":/icons/iconEdit16x16.png"));
    }
}

void COverlayText::mouseMoveEvent(QMouseEvent * e)
{
    if(doMove){
        rect.moveTopLeft(e->pos());
        rectMove = QRect(rect.topLeft()     + QPoint(2,2)  , QSize(16, 16));
        rectSize = QRect(rect.bottomRight() - QPoint(16,16), QSize(16, 16));
        rectEdit = QRect(rect.topRight()    - QPoint(36,-2), QSize(16, 16));
        rectDel  = QRect(rect.topRight()    - QPoint(18,-2), QSize(16, 16));
        theMainWindow->getCanvas()->update();
    }
    else if(doSize){
        rect.setBottomRight(e->pos());
        rectSize = QRect(rect.bottomRight() - QPoint(16,16), QSize(16, 16));
        theMainWindow->getCanvas()->update();
    }
}

void COverlayText::mousePressEvent(QMouseEvent * e)
{
    if(rectMove.contains(e->pos())){
        doMove = true;
    }
    else if(rectSize.contains(e->pos())){
        doSize = true;
    }
}

void COverlayText::mouseReleaseEvent(QMouseEvent * e)
{

    doSize = doMove = false;
}

