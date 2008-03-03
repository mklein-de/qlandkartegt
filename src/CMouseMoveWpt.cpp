/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMouseMoveWpt.cpp

  Module:

  Description:

  Created:     03/03/2008

  (C) 2008


**********************************************************************************************/

#include "CMouseMoveWpt.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "WptIcons.h"
#include "CMapDB.h"

#include <QtGui>

CMouseMoveWpt::CMouseMoveWpt(CCanvas * canvas)
    : IMouse(canvas)
    , moveWpt(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorMoveWpt"),0,0);
}

CMouseMoveWpt::~CMouseMoveWpt()
{

}

void CMouseMoveWpt::mouseMoveEvent(QMouseEvent * e)
{
    if(moveWpt){
        newPos = e->pos();
        theMainWindow->getCanvas()->update();
    }
    else{
        mouseMoveEventWpt(e);
    }
}

void CMouseMoveWpt::mousePressEvent(QMouseEvent * e)
{
    if(!moveWpt && !selWpt.isNull()){
        newPos = e->pos();
        moveWpt = true;
    }
    else if(moveWpt && !selWpt.isNull()){
        IMap& map = CMapDB::self().getMap();
        double u = e->pos().x();
        double v = e->pos().y();
        map.convertPt2Rad(u,v);
        selWpt->lon = u * RAD_TO_DEG;
        selWpt->lat = v * RAD_TO_DEG;
        moveWpt = false;
    }

    theMainWindow->getCanvas()->update();
}

void CMouseMoveWpt::mouseReleaseEvent(QMouseEvent * e)
{
}

void CMouseMoveWpt::draw(QPainter& p)
{
    if(moveWpt && !selWpt.isNull()){
        int x = newPos.x();
        int y = newPos.y();
        QPixmap icon = getWptIconByName(selWpt->icon);
        QPixmap back = QPixmap(icon.size());
        back.fill(Qt::white);
        back.setMask(icon.alphaChannel().createMaskFromColor(Qt::black));
        // draw waypoint icon
        p.drawPixmap(x-8 , y-8, back);
        p.drawPixmap(x-8 , y-7, back);
        p.drawPixmap(x-8 , y-6, back);
        p.drawPixmap(x-7 , y-8, back);

        p.drawPixmap(x-7 , y-6, back);
        p.drawPixmap(x-6 , y-8, back);
        p.drawPixmap(x-6 , y-7, back);
        p.drawPixmap(x-6 , y-6, back);

        p.drawPixmap(x-7 , y-7, icon);
    }
    else{
        drawSelWpt(p);
    }
}


