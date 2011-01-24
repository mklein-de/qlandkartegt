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
    QObject::connect(plot, SIGNAL(activePointSignal(double)), this, SLOT(activePointEvent(double)));

}


ITrackStat::~ITrackStat()
{

}


void ITrackStat::activePointEvent(double dist)
{
    qDebug() << "ITrackStat::activePointEvent" << endl;
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
            track->setPointOfFocus(idx, true, true);
            break;
        }
        if(type == eOverTime && dist < trkpt->timestamp)
        {
            track->setPointOfFocus(idx, true, true);
            break;
        }
        idx = trkpt->idx;

        ++trkpt;
    }
}


void ITrackStat::addWptTags(QVector<wpt_t>& wpts)
{

    CWptDB& wptdb = CWptDB::self();
    if(wptdb.count() == 0 || track.isNull()) return;
    wpts.resize(wptdb.count());

    IMap& map = CMapDB::self().getMap();

    QVector<wpt_t>::iterator wpt = wpts.begin();
    QMap<QString,CWpt*>::const_iterator w = wptdb.begin();

    while(wpt != wpts.end())
    {
        wpt->wpt    = (*w);
        wpt->x      = wpt->wpt->lon * DEG_TO_RAD;
        wpt->y      = wpt->wpt->lat * DEG_TO_RAD;

        map.convertRad2M(wpt->x, wpt->y);

        ++wpt; ++w;
    }

    QList<CTrack::pt_t>& trkpts                 = track->getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        if(trkpt->flags & CTrack::pt_t::eDeleted)
        {
            ++trkpt;
            continue;
        }

        double x = trkpt->lon * DEG_TO_RAD;
        double y = trkpt->lat * DEG_TO_RAD;
        map.convertRad2M(x, y);

        wpt = wpts.begin();
        while(wpt != wpts.end())
        {
            double d = (x - wpt->x) * (x - wpt->x) + (y - wpt->y) * (y - wpt->y);
            if(d < wpt->d)
            {
                wpt->d      = d;
                wpt->trkpt  = *trkpt;
            }
            ++wpt;
        }
        ++trkpt;
    }
}
