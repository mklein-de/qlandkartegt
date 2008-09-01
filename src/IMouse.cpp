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

#include "IMouse.h"
#include "CCanvas.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"
#include "COverlayDB.h"
#include "CWpt.h"
#include "CTrack.h"
#include "CMainWindow.h"
#include "CResources.h"
#include "IOverlay.h"
#include "IUnit.h"
#include "CDlgEditWpt.h"
#include "GeoMath.h"
#include <QtGui>

IMouse::IMouse(CCanvas * canvas)
: QObject(canvas)
, cursor(QPixmap(":/cursor/Arrow"))
, canvas(canvas)
, selTrkPt(0)
, doSpecialCursor(false)
{
    rectDelWpt  = QRect(0,0,16,16);
    rectMoveWpt = QRect(32,0,16,16);
    rectEditWpt = QRect(0,32,16,16);
    rectCopyWpt = QRect(32,32,16,16);
}


IMouse::~IMouse()
{

}


void IMouse::startRect(const QPoint& p)
{
    rect.setTopLeft(p);
    rect.setSize(QSize(0,0));
}


void IMouse::resizeRect(const QPoint& p)
{
    rect.setBottomRight(p);
    canvas->update();
}


void IMouse::drawRect(QPainter& p)
{
    p.setBrush(QBrush( QColor(230,230,255,100) ));
//     p.setPen(QPen(QColor(150,150,255),3));
    p.setPen(QPen(QColor(255,255,0),3));
    p.drawRect(rect);
}


void IMouse::drawSelWpt(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();
    if(!selWpt.isNull()) {
        double u = selWpt->lon * DEG_TO_RAD;
        double v = selWpt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        p.setPen(QColor(100,100,255,200));
        p.setBrush(QColor(255,255,255,200));
        p.drawEllipse(QPoint(u, v), 35, 35);

        p.save();
        p.translate(u - 24, v - 24);
        if(!selWpt->sticky){
            p.drawPixmap(rectDelWpt, QPixmap(":/icons/iconClear16x16.png"));
            p.drawPixmap(rectMoveWpt, QPixmap(":/icons/iconWptMove16x16.png"));
        }
        p.drawPixmap(rectEditWpt, QPixmap(":/icons/iconEdit16x16.png"));
        p.drawPixmap(rectCopyWpt, QPixmap(":/icons/iconClipboard16x16.png"));
        p.restore();

        QString str;
        if(selWpt->timestamp != 0x00000000 && selWpt->timestamp != 0xFFFFFFFF) {
            QDateTime time = QDateTime::fromTime_t(selWpt->timestamp);
            time.setTimeSpec(Qt::LocalTime);
            str = time.toString();
        }

        if(selWpt->ele != WPT_NOFLOAT) {
            if(str.count()) str += "\n";
            QString val, unit;
            IUnit::self().meter2elevation(selWpt->ele, val, unit);
            str += tr("elevation: %1 %2").arg(val).arg(unit);
        }

        if(selWpt->comment.count()) {
            if(str.count()) str += "\n";

            if(selWpt->comment.count() < 200) {
                str += selWpt->comment;
            }
            else {
                str += selWpt->comment.left(197) + "...";
            }

        }

        QFont           f = CResources::self().getMapFont();
        QFontMetrics    fm(f);
        QRect           r1 = fm.boundingRect(QRect(0,0,300,0), Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, str);
        r1.moveTopLeft(QPoint(u + 45, v));

        QRect           r2 = r1;
        r2.setWidth(r1.width() + 4);
        r2.moveLeft(r1.left() - 2);

        p.setPen(QColor(100,100,255,200));
        p.setBrush(QColor(255,255,255,200));
        p.drawRect(r2);

        p.setFont(CResources::self().getMapFont());
        p.setPen(Qt::darkBlue);
        p.drawText(r1, Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,str);

    }
}


void IMouse::drawSelTrkPt(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();
    if(selTrkPt && !selWpt) {
        double u = selTrkPt->lon * DEG_TO_RAD;
        double v = selTrkPt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        p.setPen(QColor(100,100,255,200));
        p.setBrush(QColor(255,255,255,200));
        p.drawEllipse(QRect(u - 5,  v - 5, 11, 11));

        QString str;
        if(selTrkPt->timestamp != 0x00000000 && selTrkPt->timestamp != 0xFFFFFFFF) {
            QDateTime time = QDateTime::fromTime_t(selTrkPt->timestamp);
            time.setTimeSpec(Qt::LocalTime);
            str = time.toString();
        }

        if(selTrkPt->ele != WPT_NOFLOAT) {
            if(str.count()) str += "\n";
            QString val, unit;
            IUnit::self().meter2elevation(selTrkPt->ele, val, unit);
            str += tr("elevation: %1 %2").arg(val).arg(unit);
        }

        QFont           f = CResources::self().getMapFont();
        QFontMetrics    fm(f);
        QRect           r1 = fm.boundingRect(QRect(0,0,300,0), Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, str);
        r1.moveTopLeft(QPoint(u + 45, v));

        QRect           r2 = r1;
        r2.setWidth(r1.width() + 4);
        r2.moveLeft(r1.left() - 2);

        p.setPen(QColor(100,100,255,200));
        p.setBrush(QColor(255,255,255,200));
        p.drawRect(r2);

        p.setFont(CResources::self().getMapFont());
        p.setPen(Qt::darkBlue);
        p.drawText(r1, Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,str);

    }
}


void IMouse::mouseMoveEventWpt(QMouseEvent * e)
{
    QPoint pos      = e->pos();
    IMap& map       = CMapDB::self().getMap();
    CWpt * oldWpt   = selWpt; selWpt = 0;

    // find the waypoint close to the cursor
    QMap<QString,CWpt*>::const_iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end()) {
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(((pos.x() - u) * (pos.x() - u) + (pos.y() - v) * (pos.y() - v)) < 1225) {
            selWpt = *wpt;
            break;
        }

        ++wpt;
    }

    // check for cursor-over-function
    if(selWpt){
        double u = selWpt->lon * DEG_TO_RAD;
        double v = selWpt->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        QPoint pt = pos - QPoint(u - 24, v - 24);

        if(rectDelWpt.contains(pt) || rectCopyWpt.contains(pt) || rectMoveWpt.contains(pt) || rectEditWpt.contains(pt)){
            if(!doSpecialCursor){
                QApplication::setOverrideCursor(Qt::PointingHandCursor);
                doSpecialCursor = true;
            }
        }
        else{
            if(doSpecialCursor){
                QApplication::restoreOverrideCursor();
                doSpecialCursor = false;
            }
        }
    }
    else {
        if(doSpecialCursor){
            QApplication::restoreOverrideCursor();
            doSpecialCursor = false;
        }
    }

    // do a canvas update on a change only
    if(oldWpt != selWpt) {
        canvas->update();
    }
}

void IMouse::mousePressEventWpt(QMouseEvent * e)
{
    if(selWpt.isNull()) return;

    IMap& map   = CMapDB::self().getMap();
    QPoint pos  = e->pos();
    double u    = selWpt->lon * DEG_TO_RAD;
    double v    = selWpt->lat * DEG_TO_RAD;
    map.convertRad2Pt(u,v);

    QPoint pt = pos - QPoint(u - 24, v - 24);
    if(rectDelWpt.contains(pt)){
        CWptDB::self().delWpt(selWpt->key(), false, true);
    }
    else if(rectMoveWpt.contains(pt)){
        canvas->setMouseMode(CCanvas::eMouseMoveWpt);

        QMouseEvent event1(QEvent::MouseMove, QPoint(u,v), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(canvas,&event1);

        QMouseEvent event2(QEvent::MouseButtonPress, QPoint(u,v), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(canvas,&event2);
    }
    else if(rectEditWpt.contains(pt)){
        CDlgEditWpt dlg(*selWpt,canvas);
        dlg.exec();
    }
    else if(rectCopyWpt.contains(pt)){
        QString position;
        GPS_Math_Deg_To_Str(selWpt->lon, selWpt->lat, position);

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(position);

        selWpt = 0;
        canvas->update();
    }
}


void IMouse::mouseMoveEventTrack(QMouseEvent * e)
{
    CTrack * track = CTrackDB::self().highlightedTrack();
    if(track == 0) return;

    CTrack::pt_t * oldTrackPt = selTrkPt;
    int d1      = 20;
    selTrkPt    = 0;

    QVector<CTrack::pt_t>& pts          = track->getTrackPoints();
    QVector<CTrack::pt_t>::iterator pt  = pts.begin();
    while(pt != pts.end()) {
        if(pt->flags & CTrack::pt_t::eDeleted) {
            ++pt; continue;
        }

        int d2 = abs(e->pos().x() - pt->px.x()) + abs(e->pos().y() - pt->px.y());

        if(d2 < d1) {
            selTrkPt = &(*pt);
            d1 = d2;
        }

        ++pt;
    }

    if(oldTrackPt != selTrkPt) {
        canvas->update();
    }

}

void IMouse::mouseMoveEventOverlay(QMouseEvent * e)
{
    IOverlay * oldOverlay = selOverlay;

    if(!selOverlay || !selOverlay->mouseActionInProgress()){

        QMap<QString, IOverlay*>::const_iterator overlay = COverlayDB::self().begin();
        while(overlay != COverlayDB::self().end()){
            if((*overlay)->isCloseEnought(e->pos())) break;
            ++overlay;
        }

        if(overlay != COverlayDB::self().end()){
            (*overlay)->select(*overlay);
            selOverlay = *overlay;
        }
        else{
            (*overlay)->select(0);
            selOverlay = 0;
        }

    }

    if(oldOverlay != selOverlay){
        if(selOverlay){
            canvas->setMouseMode(CCanvas::eMouseOverlay);
        }
        else {
            canvas->setMouseMode(CCanvas::eMouseMoveArea);
        }
        canvas->update();
    }
}

