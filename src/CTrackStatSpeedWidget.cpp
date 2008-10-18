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
#include "CTrackStatSpeedWidget.h"
#include "CPlot.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "IUnit.h"

#include <QtGui>

CTrackStatSpeedWidget::CTrackStatSpeedWidget(QWidget * parent)
: ITrackStat(parent)
, needResetZoom(true)
{
    plot->setXLabel(tr("distance [m]"));
    plot->setYLabel(tr("speed [km/h]"));

    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), this, SLOT(slotSetTrack(CTrack*)));

    slotChanged();
    plot->setLimits();
    plot->resetZoom();
}


CTrackStatSpeedWidget::~CTrackStatSpeedWidget()
{

}

void CTrackStatSpeedWidget::slotSetTrack(CTrack* track)
{
    needResetZoom = true;
}

void CTrackStatSpeedWidget::slotChanged()
{
    track = CTrackDB::self().highlightedTrack();
    if(track.isNull()) {
        plot->clear();
        return;
    }

    plot->setXLabel(tr("distance [%1]").arg(IUnit::self().baseunit));
    plot->setYLabel(tr("speed [%1]").arg(IUnit::self().speedunit));

    QPolygonF lineSpeed;
    QPolygonF marksSpeed;
    QPointF   focusSpeed;

    QPolygonF lineAvgSpeed;

    float speedfactor = IUnit::self().speedfactor;

    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()) {
        if(trkpt->flags & CTrack::pt_t::eDeleted) {
            ++trkpt; continue;
        }
        lineSpeed       << QPointF(trkpt->distance, trkpt->speed * speedfactor);
        lineAvgSpeed    << QPointF(trkpt->distance, trkpt->avgspeed * speedfactor);
        //         lineAvgSpeed    << QPointF(trkpt->distance, trkpt->velocity * speedfactor);
        if(trkpt->flags & CTrack::pt_t::eSelected) {
            marksSpeed << QPointF(trkpt->distance, trkpt->speed * speedfactor);
        }

        if(trkpt->flags & CTrack::pt_t::eFocus) {
            focusSpeed = QPointF(trkpt->distance, trkpt->speed * speedfactor);
        }

        ++trkpt;
    }

    plot->newLine(lineSpeed,focusSpeed, "speed");
    plot->addLine(lineAvgSpeed, "avg. speed");
    plot->newMarks(marksSpeed);

    plot->setLimits();
    if (needResetZoom) {
        plot->resetZoom();
        needResetZoom = false;
    }
}
