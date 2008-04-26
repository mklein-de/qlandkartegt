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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CTrackStatWidget.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "CPlot.h"

#include <QtGui>

CTrackStatWidget::CTrackStatWidget(QWidget * parent)
    : QWidget(parent)
{

    QVBoxLayout * layout  = new QVBoxLayout(this);
    setLayout(layout);
    elevation = new CPlot(this);
    layout->addWidget(elevation);

    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));

    slotChanged();
}

CTrackStatWidget::~CTrackStatWidget()
{

}


void CTrackStatWidget::slotChanged()
{
    track = CTrackDB::self().highlightedTrack();
    if(track.isNull()) {
        elevation->clear();
        return;
    }

    QPolygonF line;
    QPolygonF marks;
    QPointF   focus;

    QVector<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QVector<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()) {
        if(trkpt->flags & CTrack::pt_t::eDeleted) {
            ++trkpt; continue;
        }
        line << QPointF(trkpt->distance, trkpt->ele);
        if(trkpt->flags & CTrack::pt_t::eSelected) {
            marks << QPointF(trkpt->distance, trkpt->ele);
        }

        if(trkpt->flags & CTrack::pt_t::eFocus) {
            focus = QPointF(trkpt->distance, trkpt->ele);
        }

        ++trkpt;
    }
    elevation->setLine(line,marks,focus);

}
