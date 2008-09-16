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
#include "CTrack.h"
#include "CPlot.h"
#include "IUnit.h"


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


