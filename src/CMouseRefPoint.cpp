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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

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
    cursor = QCursor(QPixmap(":/cursors/cursorMoveRefPoint.png"),0,0);
}


CMouseRefPoint::~CMouseRefPoint()
{

}


void CMouseRefPoint::draw(QPainter& p)
{
    if(selRefPt)
    {
        IMap& map = CMapDB::self().getMap();

        double x = selRefPt->x;
        double y = selRefPt->y;
        map.convertM2Pt(x,y);

        p.drawPixmap(x - 15,y - 31,QPixmap(":/icons/iconRefPointHL31x31.png"));

    }
}


void CMouseRefPoint::mouseMoveEvent(QMouseEvent * e)
{
    IMap& map = CMapDB::self().getMap();

    mousePos = e->pos();

    if(moveMap)
    {
        map.move(oldPoint, e->pos());
        oldPoint = e->pos();
        canvas->update();
    }

    CCreateMapGeoTiff * dlg = CCreateMapGeoTiff::self();
    if(dlg == 0) return;

    if(moveRef && selRefPt)
    {
        double x = e->pos().x();
        double y = e->pos().y();
        map.convertPt2M(x,y);
        selRefPt->x = x;
        selRefPt->y = y;
        selRefPt->item->setText(CCreateMapGeoTiff::eX,tr("%1").arg((int)x));
        selRefPt->item->setText(CCreateMapGeoTiff::eY,tr("%1").arg((int)y));
        canvas->update();
    }
    else
    {
        CCreateMapGeoTiff::refpt_t * oldRefPt = selRefPt; selRefPt = 0;

        QMap<quint32,CCreateMapGeoTiff::refpt_t>& refpts         = dlg->getRefPoints();
        QMap<quint32,CCreateMapGeoTiff::refpt_t>::iterator refpt = refpts.begin();
        while(refpt != refpts.end())
        {
            double x = refpt->x;
            double y = refpt->y;
            map.convertM2Pt(x,y);

            QPoint diff = e->pos() - QPoint(x,y);
            if(diff.manhattanLength() < 100)
            {
                selRefPt = &(*refpt);
                break;
            }

            ++refpt;
        }

        if(oldRefPt != selRefPt)
        {
            canvas->update();
        }
    }
}


void CMouseRefPoint::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        if(selRefPt)
        {
            IMap& map = CMapDB::self().getMap();
            double x = e->pos().x();
            double y = e->pos().y();
            map.convertPt2M(x,y);
            selRefPt->x = x;
            selRefPt->y = y;
            selRefPt->item->setText(CCreateMapGeoTiff::eX,tr("%1").arg((int)x));
            selRefPt->item->setText(CCreateMapGeoTiff::eY,tr("%1").arg((int)y));

            CCreateMapGeoTiff * dlg = CCreateMapGeoTiff::self();
            if(dlg != 0)
            {
                dlg->selRefPointByKey(selRefPt->item->data(CCreateMapGeoTiff::eLabel,Qt::UserRole).toInt());
            }

            moveRef = true;
        }
        else
        {
            cursor = QCursor(QPixmap(":/cursors/cursorMove.png"));
            QApplication::setOverrideCursor(cursor);
            moveMap     = true;
            oldPoint    = e->pos();
        }
    }
    else if(e->button() == Qt::RightButton)
    {
        mousePos = e->pos();
        canvas->raiseContextMenu(e->pos());
    }

    canvas->update();
}


void CMouseRefPoint::mouseReleaseEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        if(moveMap)
        {
            moveMap = false;
            cursor = QCursor(QPixmap(":/cursors/cursorMoveRefPoint.png"),0,0);
            QApplication::restoreOverrideCursor();
            canvas->update();
        }
        if(moveRef)
        {
            IMap& map = CMapDB::self().getMap();
            double x = e->pos().x();
            double y = e->pos().y();
            map.convertPt2M(x,y);
            selRefPt->x = x;
            selRefPt->y = y;
            selRefPt->item->setText(CCreateMapGeoTiff::eX,tr("%1").arg((int)x));
            selRefPt->item->setText(CCreateMapGeoTiff::eY,tr("%1").arg((int)y));
            moveRef = false;

            selRefPt = 0;
            canvas->update();
        }
    }
}


void CMouseRefPoint::contextMenu(QMenu& menu)
{
    IMap& map = CMapDB::self().getMap();

    double u = mousePos.x();
    double v = mousePos.y();
    map.convertPt2Pixel(u,v);

    if(u >= 0 && v >= 0)
    {
        QString posPixel = tr("Pixel %1x%2").arg(u, 0,'f',0).arg(v,0,'f',0);
        menu.addAction(QIcon(":/icons/iconClipboard16x16.png"), posPixel, this, SLOT(slotCopyPosPixel()));

        if(pos1Pixel.x() >= 0 && pos1Pixel.y() >= 0)
        {
            double u1 = pos1Pixel.x();
            double v1 = pos1Pixel.y();

            QString posPixelSize = tr("Pos1 -> Pos %1x%2 w:%3 h:%4").arg(u1, 0,'f',0).arg(v1,0,'f',0).arg(u - u1,0,'f',0).arg(v - v1, 0,'f',0);
            menu.addAction(QIcon(":/icons/iconClipboard16x16.png"), posPixelSize, this, SLOT(slotCopyPosPixelSize()));
        }

        menu.addAction(QIcon(":/icons/wpt/flag_pin_red15x15.png"), tr("Set as Pos1"), this, SLOT(slotSetPos1()));

    }

}


void CMouseRefPoint::slotCopyPosPixel()
{
    IMap& map = CMapDB::self().getMap();

    double u = mousePos.x();
    double v = mousePos.y();

    map.convertPt2Pixel(u,v);
    QString position = QString("%1 %2").arg(u, 0,'f',0).arg(v,0,'f',0);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);

}


void CMouseRefPoint::slotCopyPosPixelSize()
{
    IMap& map = CMapDB::self().getMap();

    double u1 = pos1Pixel.x();
    double v1 = pos1Pixel.y();
    double u2 = mousePos.x();
    double v2 = mousePos.y();

    map.convertPt2Pixel(u2,v2);
    QString position = QString("%1 %2 %3 %4").arg(u1, 0,'f',0).arg(v1,0,'f',0).arg(u2 - u1, 0,'f',0).arg(v2 - v1,0,'f',0);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);

}
