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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CMouseMoveMap.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "CWptDB.h"
#include "CSearchDB.h"
#include "CTrackDB.h"
#include "CDlgEditWpt.h"
#include "GeoMath.h"
#include "CTrackToolWidget.h"

#include <QtGui>
#include "CUndoStackView.h"
#include "CMapUndoCommandMove.h"

CMouseMoveMap::CMouseMoveMap(CCanvas * parent)
: IMouse(parent)
, moveMap(false)
, leftButtonPressed(false)
, altKeyPressed(false)
, pos1(-1,-1)
{
    cursor = QCursor(QPixmap(":/cursors/cursorMoveMap.png"),0,0);
}


CMouseMoveMap::~CMouseMoveMap()
{

}


void CMouseMoveMap::mouseMoveEvent(QMouseEvent * e)
{
    mousePos = e->pos();
    if(moveMap)
    {
        //CMapDB::self().getMap().move(oldPoint, e->pos());
        CUndoStackView::getInstance()->push(new CMapUndoCommandMove(&CMapDB::self().getMap(),oldPoint, e->pos()));
        canvas->update();
    }

    oldPoint = e->pos();

    mouseMoveEventWpt(e);
    mouseMoveEventTrack(e);
    mouseMoveEventRoute(e);
    mouseMoveEventOverlay(e);
    mouseMoveEventSearch(e);
    mouseMoveEventMapSel(e);
}


void CMouseMoveMap::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {

        oldPoint = e->pos();

        CTrack * track = CTrackDB::self().highlightedTrack();

        if(!selWpt.isNull())
        {
            CWptDB::self().selWptByKey(selWpt->getKey());
            mousePressEventWpt(e);
        }
        else if(track && selTrkPt)
        {
            track->setPointOfFocus(selTrkPt->idx, true, false);
        }
        else if(!selSearch.isNull())
        {
            CSearchDB::self().selSearchByKey(selSearch->getKey());
            mousePressEventSearch(e);
        }
        else
        {
            leftButtonPressed = true;
            if (!moveMap)
            {
                CUndoStackView::getInstance()->beginMacro(tr("Move map"));
                cursor = QCursor(QPixmap(":/cursors/cursorMove.png"));
                QApplication::setOverrideCursor(cursor);
                moveMap     = true;
                CMapDB::self().getMap().fastDrawOn();
            }
        }
    }
    else if(e->button() == Qt::RightButton)
    {
        mousePos = e->pos();
        oldPoint = e->pos();
        canvas->raiseContextMenu(e->pos());
    }
}


void CMouseMoveMap::mouseReleaseEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        leftButtonPressed = false;
        if(moveMap && (!altKeyPressed))
        {
            CUndoStackView::getInstance()->endMacro();
            moveMap = false;
            CMapDB::self().getMap().fastDrawOff();
            cursor = QCursor(QPixmap(":/cursors/cursorMoveMap.png"),0,0);
            QApplication::restoreOverrideCursor();
            canvas->update();
        }


    }
}

void CMouseMoveMap::keyPressEvent(QKeyEvent * e)
{
#ifdef ALTKEY_MOVES_MAP
    if (e->key() == Qt::Key_Alt)
    {
        altKeyPressed = true;
        if (!moveMap)
        {
            CUndoStackView::getInstance()->beginMacro(tr("Move map"));
            cursor = QCursor(QPixmap(":/cursors/cursorMove.png"));
            QApplication::setOverrideCursor(cursor);
            moveMap     = true;
            CMapDB::self().getMap().fastDrawOn();
        }
    }
#endif
}

void CMouseMoveMap::keyReleaseEvent(QKeyEvent * e)
{
#ifdef ALTKEY_MOVES_MAP
    if (e->key() == Qt::Key_Alt)
    {
        altKeyPressed = false;
        if(moveMap && (!leftButtonPressed))
        {
            CUndoStackView::getInstance()->endMacro();
            moveMap = false;
            CMapDB::self().getMap().fastDrawOff();
            cursor = QCursor(QPixmap(":/cursors/cursorMoveMap.png"),0,0);
            QApplication::restoreOverrideCursor();
            canvas->update();
        }
    }
#endif
}

void CMouseMoveMap::draw(QPainter& p)
{
    drawSelWpt(p);
    drawSelTrkPt(p);
    drawSelRtePt(p);
    drawSelSearch(p);
}


void CMouseMoveMap::contextMenu(QMenu& menu)
{    
    if(!selWpt.isNull())
    {
        menu.addSeparator();
        menu.addAction(QPixmap(":/icons/iconClipboard16x16.png"),tr("Copy Pos. Waypoint"),this,SLOT(slotCopyPositionWpt()));
        menu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit Waypoint ..."),this,SLOT(slotEditWpt()));

        if(selWpt->isMovable())
        {
            menu.addAction(QPixmap(":/icons/iconWptMove16x16.png"),tr("Move Waypoint"),this,SLOT(slotMoveWpt()));
        }

        if(!selWpt->sticky)
        {
            menu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete Waypoint"),this,SLOT(slotDeleteWpt()));
        }
    }
    else
    {
        menu.addSeparator();
        menu.addAction(QPixmap(":/icons/iconAdd16x16.png"),tr("Add Waypoint ..."),this,SLOT(slotAddWpt()));
    }

    if(selTrkPt)
    {
        menu.addSeparator();
        menu.addAction(QPixmap(":/icons/iconGoogleMaps16x16.png"),tr("Open Pos. with Google Maps"),this,SLOT(slotOpenGoogleMaps())); //TODO: Google Maps right click
                menu.addAction(QPixmap(":/icons/iconClipboard16x16.png"),tr("Copy Pos. Trackpoint"),this,SLOT(slotCopyPositionTrack()));
        menu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit Track ..."),this,SLOT(slotEditTrack()));
    }
    menu.addSeparator();



    IMap& map = CMapDB::self().getMap();
    double u = mousePos.x();
    double v = mousePos.y();

    if(!map.isLonLat())
    {
        map.convertPt2M(u,v);
        QString posMeter = tr("N %1m E %2m").arg(u, 0,'f',0).arg(v,0,'f',0);
        menu.addAction(QIcon(":/icons/iconClipboard16x16.png"), posMeter, this, SLOT(slotCopyPosMeter()));
    }

    map.convertM2Rad(u,v);
    u *= RAD_TO_DEG;
    v *= RAD_TO_DEG;
    QString posDeg;
    GPS_Math_Deg_To_Str(u, v, posDeg);
    menu.addAction(QIcon(":/icons/iconClipboard16x16.png"), posDeg, this, SLOT(slotCopyPosDegree()));

    u = mousePos.x();
    v = mousePos.y();
    map.convertPt2Pixel(u,v);


    if(u >= 0 && v >= 0)
    {
        QString posPixel = tr("Pixel %1x%2").arg(u, 0,'f',0).arg(v,0,'f',0);
        menu.addAction(QIcon(":/icons/iconClipboard16x16.png"), posPixel, this, SLOT(slotCopyPosPixel()));

        if(pos1.x() >= 0 && pos1.y() >= 0)
        {
            double u1 = pos1.x();
            double v1 = pos1.y();

            QString posPixelSize = tr("Pos1 -> Pos %1x%2 w:%3 h:%4").arg(u1, 0,'f',0).arg(v1,0,'f',0).arg(u - u1,0,'f',0).arg(v - v1, 0,'f',0);
            menu.addAction(QIcon(":/icons/iconClipboard16x16.png"), posPixelSize, this, SLOT(slotCopyPosPixelSize()));
        }

        menu.addAction(QIcon(":/icons/wpt/flag_pin_red15x15.png"), tr("Set as Pos1"), this, SLOT(slotSetPos1()));

    }

}

void CMouseMoveMap::slotSetPos1()
{
    IMap& map = CMapDB::self().getMap();

    double u = mousePos.x();
    double v = mousePos.y();

    map.convertPt2Pixel(u,v);

    pos1 = QPoint(u,v);
}

void CMouseMoveMap::slotCopyPosDegree()
{
    IMap& map = CMapDB::self().getMap();
    double u = mousePos.x();
    double v = mousePos.y();

    map.convertPt2Rad(u,v);
    u *= RAD_TO_DEG;
    v *= RAD_TO_DEG;
    QString position;
    GPS_Math_Deg_To_Str(u, v, position);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);

}

void CMouseMoveMap::slotCopyPosMeter()
{
    IMap& map = CMapDB::self().getMap();
    double u = mousePos.x();
    double v = mousePos.y();

    map.convertPt2M(u,v);
    QString position = tr("N %1m E %2m").arg(u, 0,'f',0).arg(v,0,'f',0);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);
}

void CMouseMoveMap::slotCopyPosPixel()
{
    IMap& map = CMapDB::self().getMap();

    double u = mousePos.x();
    double v = mousePos.y();

    map.convertPt2Pixel(u,v);
    QString position = QString("%1 %2").arg(u, 0,'f',0).arg(v,0,'f',0);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);

}

void CMouseMoveMap::slotCopyPosPixelSize()
{
    IMap& map = CMapDB::self().getMap();

    double u1 = pos1.x();
    double v1 = pos1.y();
    double u2 = mousePos.x();
    double v2 = mousePos.y();

    map.convertPt2Pixel(u2,v2);
    QString position = QString("%1 %2 %3 %4").arg(u1, 0,'f',0).arg(v1,0,'f',0).arg(u2 - u1, 0,'f',0).arg(v2 - v1,0,'f',0);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);

}


void CMouseMoveMap::slotEditWpt()
{
    if(selWpt.isNull()) return;

    CDlgEditWpt dlg(*selWpt,canvas);
    dlg.exec();
}


void CMouseMoveMap::slotCopyPositionWpt()
{
    if(selWpt.isNull()) return;

    QString position;
    GPS_Math_Deg_To_Str(selWpt->lon, selWpt->lat, position);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);
}


void CMouseMoveMap::slotDeleteWpt()
{
    if(selWpt.isNull()) return;

    QString key = selWpt->getKey();
    CWptDB::self().delWpt(key);
}


void CMouseMoveMap::slotMoveWpt()
{
    if(selWpt.isNull()) return;
    canvas->setMouseMode(CCanvas::eMouseMoveWpt);

    double u = selWpt->lon * DEG_TO_RAD;
    double v = selWpt->lat * DEG_TO_RAD;
    CMapDB::self().getMap().convertRad2Pt(u,v);

    QMouseEvent event1(QEvent::MouseMove, QPoint(u,v), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(canvas,&event1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPoint(u,v), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(canvas,&event2);
}


void CMouseMoveMap::slotAddWpt()
{
    IMap& map = CMapDB::self().getMap();
    IMap& dem = CMapDB::self().getDEM();

    double u = mousePos.x();
    double v = mousePos.y();
    map.convertPt2Rad(u,v);
    float ele = dem.getElevation(u,v);
    CWptDB::self().newWpt(u, v, ele);

}


void CMouseMoveMap::slotCopyPositionTrack()
{
    if(!selTrkPt) return;

    QString position;
    GPS_Math_Deg_To_Str(selTrkPt->lon, selTrkPt->lat, position);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);

}


void CMouseMoveMap::slotEditTrack()
{
    if(!selTrkPt) return;

    CTrackToolWidget * toolview = CTrackDB::self().getToolWidget();
    if(toolview) toolview->slotEdit();

    CTrack * track = CTrackDB::self().highlightedTrack();
    if(track)
    {
        track->setPointOfFocus(selTrkPt->idx, true, false);
    }
}

void CMouseMoveMap::slotOpenGoogleMaps()	//TODO: Open Google Maps
{
    QString position;
    GPS_Math_Deg_To_Str(selTrkPt->lon, selTrkPt->lat, position);


    QDateTime utime = QDateTime::fromTime_t(selTrkPt->timestamp);
    utime.setTimeSpec(Qt::LocalTime);
    QString time = utime.toString();

    QDesktopServices::openUrl(QUrl("http://maps.google.com/maps?t=h&z=18&om=1&q="+position+"("+time+")", QUrl::TolerantMode));
}

