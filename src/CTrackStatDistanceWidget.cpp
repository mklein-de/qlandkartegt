/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#include "CTrackStatDistanceWidget.h"
#include "CPlot.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "IUnit.h"

#include <QtGui>


CTrackStatDistanceWidget::CTrackStatDistanceWidget(QWidget * parent)
    : ITrackStat(eOverTime, parent)
    , needResetZoom(true)
{
    plot->setXLabel(tr("time [h]"));
    plot->setYLabel(tr("distance [m]"));

    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), this, SLOT(slotSetTrack(CTrack*)));

    slotChanged();
    plot->setLimits();
    plot->resetZoom();

}

CTrackStatDistanceWidget::~CTrackStatDistanceWidget()
{

}

void CTrackStatDistanceWidget::slotSetTrack(CTrack* track)
{
    needResetZoom = true;

}

void CTrackStatDistanceWidget::slotChanged()
{
    track = CTrackDB::self().highlightedTrack();
    if(track.isNull())
    {
        plot->clear();
        return;
    }

    QPolygonF lineDist;
    QPolygonF marksDist;
    QPointF   focusDist;

    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        if(trkpt->flags & CTrack::pt_t::eDeleted)
        {
            ++trkpt; continue;
        }

        lineDist  << QPointF((double)trkpt->timestamp, trkpt->distance);

        if(trkpt->flags & CTrack::pt_t::eSelected)
        {
            marksDist << QPointF((double)trkpt->timestamp, trkpt->distance);
        }

        if(trkpt->flags & CTrack::pt_t::eFocus)
        {
            focusDist = QPointF((double)trkpt->timestamp, trkpt->distance);
        }

        ++trkpt;
    }

    QVector<wpt_t> wpts;

    plot->clear();
    addWptTags(wpts);

    QVector<wpt_t>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        if(wpt->d < 400)
        {
            CPlotData::point_t tag;
            tag.point = QPointF((double)wpt->trkpt.timestamp, wpt->trkpt.distance);
            tag.icon  = wpt->wpt->getIcon();
            tag.label = wpt->wpt->getName();
            plot->addTag(tag);

        }
        ++wpt;
    }



    plot->newLine(lineDist,focusDist, "dist.");
    plot->newMarks(marksDist);

    plot->setLimits();
    if (needResetZoom)
    {
        plot->resetZoom();
        needResetZoom = false;
    }

}