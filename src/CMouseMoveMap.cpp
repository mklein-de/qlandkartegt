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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CMouseMoveMap.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "CWptDB.h"
#include "CTrackDB.h"
#include "CDlgEditWpt.h"
#include "GeoMath.h"
#include "CTrackToolWidget.h"

#include <QtGui>

CMouseMoveMap::CMouseMoveMap(CCanvas * parent)
: IMouse(parent)
, moveMap(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorMoveMap"),0,0);
}


CMouseMoveMap::~CMouseMoveMap()
{

}


void CMouseMoveMap::mouseMoveEvent(QMouseEvent * e)
{
    mousePos = e->pos();

    if(moveMap) {
        CMapDB::self().getMap().move(oldPoint, e->pos());
        oldPoint = e->pos();
        canvas->update();
    }

    mouseMoveEventWpt(e);
    mouseMoveEventTrack(e);
    mouseMoveEventOverlay(e);
    mouseMoveEventSearch(e);
}


void CMouseMoveMap::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {

        CTrack * track = CTrackDB::self().highlightedTrack();

        if(!selWpt.isNull()) {
            CWptDB::self().selWptByKey(selWpt->key());
            mousePressEventWpt(e);
        }
        else if(track && selTrkPt) {
            track->setPointOfFocus(selTrkPt->idx);
        }
        else if(!selSearch.isNull()){
            mousePressEventSearch(e);
        }
        else{
            cursor = QCursor(QPixmap(":/cursors/cursorMove"));
            QApplication::setOverrideCursor(cursor);
            moveMap     = true;
            oldPoint    = e->pos();
        }
    }
    else if(e->button() == Qt::RightButton) {
        canvas->raiseContextMenu(e->pos());
    }
}

void CMouseMoveMap::mouseReleaseEvent(QMouseEvent * e)
{
    if(moveMap && (e->button() == Qt::LeftButton)) {
        moveMap = false;
        cursor = QCursor(QPixmap(":/cursors/cursorMoveMap"),0,0);
        QApplication::restoreOverrideCursor();
        canvas->update();
    }
}

void CMouseMoveMap::draw(QPainter& p)
{
    drawSelWpt(p);
    drawSelTrkPt(p);
    drawSelSearch(p);
}

void CMouseMoveMap::contextMenu(QMenu& menu)
{
    if(!selWpt.isNull()){
        menu.addSeparator();
        menu.addAction(QPixmap(":/icons/iconClipboard16x16.png"),tr("Copy Pos. Waypoint"),this,SLOT(slotCopyPositionWpt()));
        menu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit Waypoint ..."),this,SLOT(slotEditWpt()));
        if(!selWpt->sticky){
            menu.addAction(QPixmap(":/icons/iconWptMove16x16.png"),tr("Move Waypoint"),this,SLOT(slotMoveWpt()));
            menu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete Waypoint"),this,SLOT(slotDeleteWpt()));
        }
    }
    else{
        menu.addSeparator();
        menu.addAction(QPixmap(":/icons/iconAdd16x16.png"),tr("Add Waypoint ..."),this,SLOT(slotAddWpt()));
    }

    if(selTrkPt){
        menu.addSeparator();
        menu.addAction(QPixmap(":/icons/iconClipboard16x16.png"),tr("Copy Pos. Trackpoint"),this,SLOT(slotCopyPositionTrack()));
        menu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit Track ..."),this,SLOT(slotEditTrack()));
    }
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

    QString key = selWpt->key();
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
    if(track) {
        track->setPointOfFocus(selTrkPt->idx);
    }
}
