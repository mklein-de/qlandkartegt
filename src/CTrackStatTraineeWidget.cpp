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
#include "CTrackStatTraineeWidget.h"
#include "CPlot.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "IUnit.h"

#include <QtGui>

CTrackStatTraineeWidget::CTrackStatTraineeWidget(QWidget * parent)
: ITrackStat(parent)
, needResetZoom(true)
{
    plot->setXLabel(tr("distance [m]"));
    plot->setYLabel(tr("heartrate [bpm]"));

    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), this, SLOT(slotSetTrack(CTrack*)));

    slotChanged();
    plot->setLimits();
    plot->resetZoom();
}


CTrackStatTraineeWidget::~CTrackStatTraineeWidget()
{

}


void CTrackStatTraineeWidget::slotSetTrack(CTrack* track)
{
    needResetZoom = true;
}


void CTrackStatTraineeWidget::slotChanged()
{
    track = CTrackDB::self().highlightedTrack();
    if(track.isNull()) {
        plot->clear();
        return;
    }

    plot->setXLabel(tr("distance [%1]").arg(IUnit::self().baseunit));
    plot->setYLabel(tr("heardrate [bpm]"));

    QPolygonF heartRate;
    QPolygonF slopeRate;
    QPointF   focusSpeed;

    QPolygonF lineAvgSpeed;

    //     float speedfactor = IUnit::self().speedfactor;

    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()) {
        if(trkpt->flags & CTrack::pt_t::eDeleted) {
            ++trkpt; continue;
        }
        //qDebug() << trkpt->heartReateBpm;
        heartRate       << QPointF(trkpt->distance, trkpt->heartReateBpm);
        slopeRate       << QPointF(trkpt->distance, trkpt->slope + 100.0);
        //      lineAvgSpeed    << QPointF(trkpt->distance, trkpt->avgspeed * speedfactor);
        //      if(trkpt->flags & CTrack::pt_t::eSelected) {
        //          marksSpeed << QPointF(trkpt->distance, trkpt->speed * speedfactor);
        //      }

        if(trkpt->flags & CTrack::pt_t::eFocus) {
            focusSpeed = QPointF(trkpt->distance, trkpt->heartReateBpm);
        }

        ++trkpt;
    }

    plot->newLine(heartRate,focusSpeed, "speed");
    plot->addLine(slopeRate, "slope");
    // plot->newMarks(marksSpeed);

    plot->setLimits();
    if (needResetZoom) {
        plot->resetZoom();
        needResetZoom = false;
    }
}
