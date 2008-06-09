/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        COverlayTextBox.cpp

  Module:

  Description:

  Created:     06/09/2008

  (C) 2008


**********************************************************************************************/

#include "COverlayTextBox.h"
#include "CMapDB.h"

#include <QtGui>

COverlayTextBox::COverlayTextBox(const QPointF& anchor, const QRect& rect, QObject * parent)
: IOverlay(parent, "TextBox")
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

