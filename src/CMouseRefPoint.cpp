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
, moveRef(false)
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
    IMap& map = CMapDB::self().getMap();

    if(moveMap) {
        map.move(oldPoint, e->pos());
        oldPoint = e->pos();
        canvas->update();
    }

    CCreateMapGeoTiff * dlg = CCreateMapGeoTiff::self();
    if(dlg == 0) return;

    if(moveRef && selRefPt){
        double x = e->pos().x();
        double y = e->pos().y();
        map.convertPt2M(x,y);
        selRefPt->x = x;
        selRefPt->y = y;
        selRefPt->item->setText(CCreateMapGeoTiff::eX,tr("%1").arg((int)x));
        selRefPt->item->setText(CCreateMapGeoTiff::eY,tr("%1").arg((int)y));
        canvas->update();
    }
    else {
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
}

void CMouseRefPoint::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        if(selRefPt){
            IMap& map = CMapDB::self().getMap();
            double x = e->pos().x();
            double y = e->pos().y();
            map.convertPt2M(x,y);
            selRefPt->x = x;
            selRefPt->y = y;
            selRefPt->item->setText(CCreateMapGeoTiff::eX,tr("%1").arg((int)x));
            selRefPt->item->setText(CCreateMapGeoTiff::eY,tr("%1").arg((int)y));
            moveRef = true;
        }
        else{
            cursor = QCursor(QPixmap(":/cursors/cursorMove"));
            QApplication::setOverrideCursor(cursor);
            moveMap     = true;
            oldPoint    = e->pos();
        }
    }

    canvas->update();
}

void CMouseRefPoint::mouseReleaseEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        if(moveMap){
            moveMap = false;
            cursor = QCursor(QPixmap(":/cursors/cursorMoveRefPoint"),0,0);
            QApplication::restoreOverrideCursor();
            canvas->update();
        }
        if(moveRef){
            IMap& map = CMapDB::self().getMap();
            double x = e->pos().x();
            double y = e->pos().y();
            map.convertPt2M(x,y);
            selRefPt->x = x;
            selRefPt->y = y;
            selRefPt->item->setText(CCreateMapGeoTiff::eX,tr("%1").arg((int)x));
            selRefPt->item->setText(CCreateMapGeoTiff::eY,tr("%1").arg((int)y));
            moveRef = false;

            canvas->update();
        }
    }
}
