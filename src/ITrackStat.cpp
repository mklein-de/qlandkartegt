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

#include "ITrackStat.h"
#include "CTrackDB.h"
#include "CPlot.h"
#include "IUnit.h"
#include "CWptDB.h"
#include "IMap.h"
#include "CMapDB.h"

#define SPACING 9

#include <QtGui>

ITrackStat::ITrackStat(type_e type, QWidget * parent)
: QWidget(parent)
, type(type)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setupUi(this);

    layout()->setSpacing(SPACING);

    if(type == eOverDistance)
    {
        plot = new CPlot(CPlotData::eLinear, CPlot::eNormal, this);
    }
    else
    {
        plot = new CPlot(CPlotData::eTime, CPlot::eNormal, this);
    }
    layout()->addWidget(plot);
    QObject::connect(plot, SIGNAL(sigActivePoint(double)), this, SLOT(slotActivePoint(double)));
    QObject::connect(plot, SIGNAL(sigSetWaypoint(double)), this, SLOT(slotSetWaypoint(double)));

}


ITrackStat::~ITrackStat()
{

}


void ITrackStat::slotActivePoint(double dist)
{
    if(track.isNull()) return;
    if(plot == 0) return;
    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    quint32 idx = 0;
    while(trkpt != trkpts.end())
    {
        if(trkpt->flags & CTrack::pt_t::eDeleted)
        {
            ++trkpt; continue;
        }

        if(type == eOverDistance && dist < trkpt->distance)
        {
            track->setPointOfFocus(idx, CTrack::e3Way, true);
            break;
        }
        if(type == eOverTime && dist < trkpt->timestamp)
        {
            track->setPointOfFocus(idx, CTrack::e3Way, true);
            break;
        }
        idx = trkpt->idx;

        ++trkpt;
    }
}


void ITrackStat::slotSetWaypoint(double dist)
{
    if(track.isNull()) return;

    IMap& dem = CMapDB::self().getDEM();
    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt = trkpts.begin();

    while(trkpt != trkpts.end())
    {
        if(trkpt->flags & CTrack::pt_t::eDeleted)
        {
            ++trkpt; continue;
        }

        if(type == eOverDistance && dist < trkpt->distance)
        {
            plot->setSelTrackPoint(&(*trkpt));

            double lon = trkpt->lon * DEG_TO_RAD;
            double lat = trkpt->lat * DEG_TO_RAD;
            double ele = dem.getElevation(lon, lat);
            if(ele == WPT_NOFLOAT)
            {
                ele = trkpt->ele;
            }
            CWptDB::self().newWpt(lon, lat, ele, CWptDB::self().getNewWptName());
            break;
        }
        if(type == eOverTime && dist < trkpt->timestamp)
        {
            plot->setSelTrackPoint(&(*trkpt));
            double lon = trkpt->lon * DEG_TO_RAD;
            double lat = trkpt->lat * DEG_TO_RAD;
            double ele = dem.getElevation(lon, lat);
            if(ele == WPT_NOFLOAT)
            {
                ele = trkpt->ele;
            }
            CWptDB::self().newWpt(lon, lat, ele, CWptDB::self().getNewWptName());
            break;
        }

        ++trkpt;
    }
}
