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

#include "COverlayTextBox.h"
#include "CMapDB.h"

#include <QtGui>

COverlayTextBox::COverlayTextBox(const QPointF& anchor, const QRect& rect, QObject * parent)
: IOverlay(parent, "TextBox", QPixmap(":/icons/iconTextBox16x16"))
, u(anchor.x())
, v(anchor.y())
, rect(rect)
{

}

COverlayTextBox::~COverlayTextBox()
{

}

QPolygon COverlayTextBox::polygon(int x, int y, const QRect& r)
{
    QPolygon poly1, poly2;
    poly1 << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();

    if(!r.contains(x,y)){
        int w = (r.width()>>1) - 50;
        int h = (r.height()>>1) - 50;

        w = w > 100 ? 100 : w;
        h = h > 100 ? 100 : h;

        if(x < r.left()){
            poly2 << QPoint(x,y) << (r.center() + QPoint(0,-h)) << (r.center() + QPoint(0,h));
        }
        else if(r.right() < x){
            poly2 << QPoint(x,y) << (r.center() + QPoint(0,-h)) << (r.center() + QPoint(0,h));
        }
        else if(y < r.top()){
            poly2 << QPoint(x,y) << (r.center() + QPoint(-w,0)) << (r.center() + QPoint(w,0));
        }
        else if(r.bottom() < y){
            poly2 << QPoint(x,y) << (r.center() + QPoint(-w,0)) << (r.center() + QPoint(w,0));
        }

        poly1 = poly1.united(poly2);
    }

    return poly1;
}

void COverlayTextBox::draw(QPainter& p)
{
    double x = u;
    double y = v;

    CMapDB::self().getMap().convertRad2Pt(x,y);

    p.setBrush(Qt::white);
    p.setPen(Qt::black);
    p.drawPolygon(polygon(x,y,rect));
}

