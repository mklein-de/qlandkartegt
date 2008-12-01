/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CMouseColorPicker.cpp

  Module:

  Description:

  Created:     12/01/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CMouseColorPicker.h"
#include "CMapDB.h"
#include "IMap.h"
#include "CMainWindow.h"

#include <QtGui>

CMouseColorPicker::CMouseColorPicker(CCanvas * canvas)
: IMouse(canvas)
, color(Qt::NoPen)
, selected(Qt::NoPen)
{
    cursor = QCursor(QPixmap(":/cursors/cursorColorChooser"),3,30);
}

CMouseColorPicker::~CMouseColorPicker()
{

}

void CMouseColorPicker::draw(QPainter& p)
{
    p.setPen(Qt::black);


    if(selected == Qt::NoPen){
        p.setBrush(color);
        p.drawRect(50,50,100,100);
    }
    else{
        p.setBrush(color);
        p.drawRect(50,50,50,100);
        p.setBrush(selected);
        p.drawRect(100,50,50,100);
    }
}

void CMouseColorPicker::mouseMoveEvent(QMouseEvent * e)
{
    IMap& map           = CMapDB::self().getMap();
    const QImage& img   = map.getBuffer();

    QColor c = img.pixel(e->pos());

    if(c != color){
        color = c;
        theMainWindow->getCanvas()->update();
    }
}

void CMouseColorPicker::mousePressEvent(QMouseEvent * e)
{
    selected = color;
    theMainWindow->getCanvas()->update();
}

void CMouseColorPicker::mouseReleaseEvent(QMouseEvent * e)
{

}


