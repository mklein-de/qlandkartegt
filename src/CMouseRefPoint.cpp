/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMouseRefPoint.cpp

  Module:

  Description:

  Created:     03/07/2008

  (C) 2008


**********************************************************************************************/

#include "CMouseRefPoint.h"
#include "CCanvas.h"
#include "CMapDB.h"

#include <QtGui>


CMouseRefPoint::CMouseRefPoint(CCanvas * canvas)
: IMouse(canvas)
, moveMap(false)
, selRefPt(0)
{
    cursor = QCursor(QPixmap(":/cursors/cursorMoveRefPoint"),0,0);
}

CMouseRefPoint::~CMouseRefPoint()
{

}

void CMouseRefPoint::draw(QPainter& p)
{
    if(selRefPt){
        IMap& map = CMapDB::self().getMap();

        double x = selRefPt->x;
        double y = selRefPt->y;
        map.convertM2Pt(x,y);

        p.drawPixmap(x - 15,y - 31,QPixmap(":/icons/iconRefPointHL31x31"));

    }
}

void CMouseRefPoint::mouseMoveEvent(QMouseEvent * e)
{
    if(moveMap) {
        CMapDB::self().getMap().move(oldPoint, e->pos());
        oldPoint = e->pos();
        canvas->update();
    }

    CCreateMapGeoTiff * dlg = CCreateMapGeoTiff::self();
    if(dlg == 0) return;

    IMap& map = CMapDB::self().getMap();
    CCreateMapGeoTiff::refpt_t * oldRefPt = selRefPt; selRefPt = 0;

    QMap<quint32,CCreateMapGeoTiff::refpt_t>& refpts         = dlg->getRefPoints();
    QMap<quint32,CCreateMapGeoTiff::refpt_t>::iterator refpt = refpts.begin();
    while(refpt != refpts.end()){
        double x = refpt->x;
        double y = refpt->y;
        map.convertM2Pt(x,y);

        QPoint diff = e->pos() - QPoint(x,y);
        if(diff.manhattanLength() < 30) {
            selRefPt = &(*refpt);
            break;
        }

        ++refpt;
    }

    if(oldRefPt != selRefPt) {
        canvas->update();
    }

}

void CMouseRefPoint::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        cursor = QCursor(QPixmap(":/cursors/cursorMove"));
        QApplication::setOverrideCursor(cursor);
        moveMap     = true;
        oldPoint    = e->pos();
    }
}

void CMouseRefPoint::mouseReleaseEvent(QMouseEvent * e)
{
    if(moveMap && (e->button() == Qt::LeftButton)) {
        moveMap = false;
        cursor = QCursor(QPixmap(":/cursors/cursorMoveRefPoint"),0,0);
        QApplication::restoreOverrideCursor();
        canvas->update();
    }
}
