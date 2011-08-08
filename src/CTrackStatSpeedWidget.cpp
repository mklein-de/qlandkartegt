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
#include "CTrackStatSpeedWidget.h"
#include "CPlot.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "IUnit.h"
#include "GeoMath.h"

#include <QtGui>

CTrackStatSpeedWidget::CTrackStatSpeedWidget(type_e type, QWidget * parent)
: ITrackStat(type, parent)
, needResetZoom(true)
{
    if(type == eOverDistance)
    {
        plot->setXLabel(tr("distance [m]"));
    }
    else
    {
        plot->setXLabel(tr("time [h]"));
    }
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

#define MEDIAN_FLT_LEN 15
void CTrackStatSpeedWidget::slotChanged()
{
    track = CTrackDB::self().highlightedTrack();
    if(track.isNull())
    {
        plot->clear();
        return;
    }

    if(type == eOverDistance)
    {
        plot->setXLabel(tr("distance [%1]").arg(IUnit::self().baseunit));
    }
    else
    {
        plot->setXLabel(tr("time [h]"));
    }

    plot->setYLabel(tr("speed [%1]").arg(IUnit::self().speedunit));

    QPolygonF lineSpeed;
    QPolygonF marksSpeed;
    QPointF   focusSpeed;

    QPolygonF lineAvgSpeed;

    float speedfactor = IUnit::self().speedfactor;

    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt0   = trkpts.begin();
    QList<CTrack::pt_t>::const_iterator trkpt1  = trkpts.begin() + 1;
    QList<float> speed;

    while(trkpt1 != trkpts.end())
    {      
        float medSpeed;
        double a2, a1;
        XY p1,p2;
        p1.u     = DEG_TO_RAD * trkpt0->lon;
        p1.v     = DEG_TO_RAD * trkpt0->lat;
        p2.u     = DEG_TO_RAD * trkpt1->lon;
        p2.v     = DEG_TO_RAD * trkpt1->lat;
        double d = distance(p1,p2,a1,a2);
        int dt   = trkpt1->timestamp -  trkpt0->timestamp;

        speed << (dt ? d/dt : 0.0);
        if(speed.size() == MEDIAN_FLT_LEN)
        {
            QList<float> tmp = speed;
            qSort(tmp);
            medSpeed = tmp[MEDIAN_FLT_LEN>>1];

            speed.pop_front();
        }
        else
        {
            medSpeed = 0;
        }


        if(trkpt0->flags & CTrack::pt_t::eDeleted)
        {
            ++trkpt0; ++trkpt1; continue;
        }

        lineSpeed       << QPointF(type == eOverDistance ? trkpt0->distance : (double)trkpt0->timestamp, trkpt0->speed * speedfactor);
        lineAvgSpeed    << QPointF(type == eOverDistance ? trkpt0->distance : (double)trkpt0->timestamp, medSpeed * speedfactor);

        if(trkpt0->flags & CTrack::pt_t::eSelected)
        {
            marksSpeed << QPointF(type == eOverDistance ? trkpt0->distance : (double)trkpt0->timestamp, trkpt0->speed * speedfactor);
        }

        if(trkpt0->flags & CTrack::pt_t::eFocus)
        {
            focusSpeed = QPointF(type == eOverDistance ? trkpt0->distance : (double)trkpt0->timestamp, trkpt0->speed * speedfactor);
        }

        ++trkpt0; ++trkpt1;
    }

    plot->newLine(lineSpeed,focusSpeed, "speed");
    plot->addLine(lineAvgSpeed, "med. speed");
    plot->newMarks(marksSpeed);

    plot->setLimits();
    if (needResetZoom)
    {
        plot->resetZoom();
        needResetZoom = false;
    }

}
