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

#define SPACING 9

CTrackStatWidget::CTrackStatWidget(QWidget * parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setupUi(this);

    layout()->setSpacing(SPACING);

    elevation = new CPlot(this);
    elevation->setXLabel(tr("distance [m]"));
    elevation->setYLabel(tr("alt. [m]"));
    layout()->addWidget(elevation);

    speed = new CPlot(this);
    speed->setXLabel(tr("distance [m]"));
    speed->setYLabel(tr("speed [km/h]"));
    layout()->addWidget(speed);

    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));

    slotChanged();

    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked(bool)), this, SLOT(close()));
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

    QPolygonF lineElev;
    QPolygonF marksElev;
    QPointF   focusElev;

    QPolygonF lineSpeed;
    QPolygonF marksSpeed;
    QPointF   focusSpeed;

    QVector<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QVector<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()) {
        if(trkpt->flags & CTrack::pt_t::eDeleted) {
            ++trkpt; continue;
        }
        lineElev  << QPointF(trkpt->distance, trkpt->ele);
        lineSpeed << QPointF(trkpt->distance, trkpt->speed);
        if(trkpt->flags & CTrack::pt_t::eSelected) {
            marksElev  << QPointF(trkpt->distance, trkpt->ele);
            marksSpeed << QPointF(trkpt->distance, trkpt->speed);
        }

        if(trkpt->flags & CTrack::pt_t::eFocus) {
            focusElev  = QPointF(trkpt->distance, trkpt->ele);
            focusSpeed = QPointF(trkpt->distance, trkpt->speed);
        }

        ++trkpt;
    }
    elevation->setLine(lineElev,marksElev,focusElev);
    speed->setLine(lineSpeed,marksSpeed,focusSpeed);

}

void CTrackStatWidget::mousePressEvent(QMouseEvent * e)
{
    if(track.isNull()) return;

    if(e->button() == Qt::LeftButton) {
        QPoint pos = e->pos();
        CPlot * plot = 0;

        // test for elevation graph
        if(elevation->rect().contains(pos)){
            plot = elevation;
        }

        // adjust position to speed graph
        pos.setY(pos.y() - elevation->rect().height());

        // test for speed graph
        if(speed->rect().contains(pos)){
            plot = speed;
        }


        if(plot == 0) return;

        double dist = plot->getXValByPixel(pos.x() - SPACING);
        QVector<CTrack::pt_t>& trkpts = track->getTrackPoints();
        QVector<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
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

