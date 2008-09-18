/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        ITrackStat.cpp

  Module:

  Description:

  Created:     09/15/2008

  (C) 2008 DSP Solutions. All rights reserved.


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

ITrackStat::ITrackStat(QWidget * parent)
: QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setupUi(this);

    layout()->setSpacing(SPACING);

    plot = new CPlot(this);
    layout()->addWidget(plot);


}

ITrackStat::~ITrackStat()
{

}

void ITrackStat::mousePressEvent(QMouseEvent * e)
{
    if(track.isNull()) return;

    if(e->button() == Qt::LeftButton) {
        QPoint pos = e->pos();

        if(plot == 0) return;

        double dist = plot->getXValByPixel(pos.x() - SPACING);
        QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
        QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
        quint32 idx = 0;
        while(trkpt != trkpts.end()) {
            if(trkpt->flags & CTrack::pt_t::eDeleted) {
                ++trkpt; continue;
            }

            if(dist < trkpt->distance) {
                track->setPointOfFocus(idx);
                break;
            }
            idx = trkpt->idx;

            ++trkpt;
        }
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

    while(wpt != wpts.end()){
        wpt->wpt    = (*w);
        wpt->x      = wpt->wpt->lon * DEG_TO_RAD;
        wpt->y      = wpt->wpt->lat * DEG_TO_RAD;

        map.convertRad2M(wpt->x, wpt->y);

        ++wpt; ++w;
    }



    QList<CTrack::pt_t>& trkpts                 = track->getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end()){
        if(trkpt->flags & CTrack::pt_t::eDeleted){
            ++trkpt;
            continue;
        }

        double x = trkpt->lon * DEG_TO_RAD;
        double y = trkpt->lat * DEG_TO_RAD;
        map.convertRad2M(x, y);

        wpt = wpts.begin();
        while(wpt != wpts.end()){
            double d = (x - wpt->x) * (x - wpt->x) + (y - wpt->y) * (y - wpt->y);
            if(d < wpt->d){
                wpt->d      = d;
                wpt->trkpt  = *trkpt;
            }
            ++wpt;
        }
        ++trkpt;
    }
}

