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

#include "CTrackStatProfileWidget.h"
#include "CPlot.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "IUnit.h"


#include <QtGui>

CTrackStatProfileWidget::CTrackStatProfileWidget(QWidget * parent)
: ITrackStat(parent)
{

    plot->setXLabel(tr("distance [m]"));
    plot->setYLabel(tr("alt. [m]"));

    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));

    slotChanged();

}

CTrackStatProfileWidget::~CTrackStatProfileWidget()
{

}

void CTrackStatProfileWidget::slotChanged()
{
    track = CTrackDB::self().highlightedTrack();
    if(track.isNull()) {
        plot->clear();
        return;
    }

    plot->setXLabel(tr("distance [%1]").arg(IUnit::self().baseunit));
    plot->setYLabel(tr("alt. [%1]").arg(IUnit::self().baseunit));


    QPolygonF lineDEM;

    QPolygonF lineElev;
    QPolygonF marksElev;
    QPointF   focusElev;

    float basefactor = IUnit::self().basefactor;

    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()) {
        if(trkpt->flags & CTrack::pt_t::eDeleted) {
            ++trkpt; continue;
        }
        if(trkpt->dem != WPT_NOFLOAT){
            lineDEM << QPointF(trkpt->distance, trkpt->dem * basefactor);
        }
        lineElev    << QPointF(trkpt->distance, trkpt->ele * basefactor);
        if(trkpt->flags & CTrack::pt_t::eSelected) {
            marksElev  << QPointF(trkpt->distance, trkpt->ele * basefactor);
        }

        if(trkpt->flags & CTrack::pt_t::eFocus) {
            focusElev  = QPointF(trkpt->distance, trkpt->ele * basefactor);
        }

        ++trkpt;
    }

    plot->newLine(lineElev,focusElev, "GPS");
    plot->newMarks(marksElev);
    if(!lineDEM.isEmpty()){
        plot->addLine(lineDEM, "DEM");
    }
}


