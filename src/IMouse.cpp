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
#include <QtGui>

IMouse::IMouse(CCanvas * canvas)
: QObject(canvas)
, cursor(QPixmap(":/cursor/Arrow"))
, canvas(canvas)
, selTrkPt(0)
{

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
        p.drawEllipse(QRect(u - 11,  v - 11, 22, 22));

        QString str;
        if(selWpt->timestamp != 0x00000000 && selWpt->timestamp != 0xFFFFFFFF) {
            QDateTime time = QDateTime::fromTime_t(selWpt->timestamp);
            time.setTimeSpec(Qt::LocalTime);
            str = time.toString();
        }

        if(selWpt->ele != WPT_NOFLOAT) {
            if(str.count()) str += "\n";
            str += tr("elevation: %1 m").arg(selWpt->ele,0,'f',0);
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
            str += tr("elevation: %1 m").arg(selTrkPt->ele,0,'f',0);
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
    IMap& map = CMapDB::self().getMap();
    CWpt * oldWpt = selWpt; selWpt = 0;

    QMap<QString,CWpt*>::const_iterator wpt = CWptDB::self().begin();
    while(wpt != CWptDB::self().end()) {
        double u = (*wpt)->lon * DEG_TO_RAD;
        double v = (*wpt)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        QPoint diff = e->pos() - QPoint(u,v);
        if(diff.manhattanLength() < 15) {
            selWpt = *wpt;
            break;
        }

        ++wpt;
    }

    if(oldWpt != selWpt) {
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

