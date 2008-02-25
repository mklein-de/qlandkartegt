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

#include "CTrack.h"
#include "GeoMath.h"

#include <QtGui>

const QColor CTrack::colors[] =
{
     Qt::black                   // 0
    ,Qt::darkRed                 // 1
    ,Qt::darkGreen               // 2
    ,Qt::darkYellow              // 3
    ,Qt::darkBlue                // 4
    ,Qt::darkMagenta             // 5
    ,Qt::darkCyan                // 6
    ,Qt::gray                    // 7
    ,Qt::darkGray                // 8
    ,Qt::red                     // 9
    ,Qt::green                   // 10
    ,Qt::yellow                  // 11
    ,Qt::blue                    // 12
    ,Qt::magenta                 // 13
    ,Qt::cyan                    // 14
    ,Qt::white                   // 15
    ,Qt::transparent             // 16
};


CTrack::CTrack(QObject * parent)
    : QObject(parent)
    , timestamp(QDateTime::currentDateTime().toUTC().toTime_t ())
    , color(Qt::darkBlue)
    , colorIdx(4)
    , highlight(false)
{

}

CTrack::~CTrack()
{

}

void CTrack::setColor(unsigned i)
{
    if(i>16) i = 4;
    colorIdx    = i;
    color       = colors[i];
}

void CTrack::genKey()
{
    _key_ = QString("%1%2").arg(timestamp).arg(name);
}

const QString& CTrack::key()
{
    if(_key_.isEmpty()) genKey();
    return _key_;
}

CTrack& CTrack::operator<<(pt_t& pt)
{
    track.push_back(pt);
    track.last().idx     = track.size() - 1;
    track.last().flags  &= ~pt_t::eCursor;
    track.last().flags  &= ~pt_t::eFocus;
    track.last().flags  &= ~pt_t::eSelected;

    return *this;
}

void CTrack::rebuild(bool reindex)
{
    quint32 t1 = 0, t2 = 0;
    QVector<pt_t>::iterator pt1 = track.begin();
    QVector<pt_t>::iterator pt2 = track.begin();

    totalTime       = 0;
    totalDistance   = 0;

    // reindex track if desired
    if(reindex) {
        quint32 idx = 0;
        while(pt1 != track.end()) {
            pt1->idx = idx++;
            ++pt1;
        }
        pt1 = track.begin();
    }

    // skip leading deleted points
    while((pt1->flags & pt_t::eDeleted) && (pt1 != track.end())) {
        pt1->azimuth    = 0;
        pt1->delta      = 0;
        pt1->speed      = -1;
        pt1->distance   = 0;
        ++pt1; ++pt2;
    }

    // no points at all?
    if(pt1 == track.end()) {
        emit sigChanged();
        return;
    }
    // reset first valid point
    pt1->azimuth    = 0;
    pt1->delta      = 0;
    pt1->speed      = -1;
    pt1->distance   = 0;
    t1              = pt1->time;

    // process track
    while(++pt2 != track.end()) {

        // reset deleted points
        if(pt2->flags & pt_t::eDeleted) {
            pt2->azimuth    = 0;
            pt2->delta      = 0;
            pt2->speed      = -1;
            pt2->distance   = 0;
            continue;
        }

        int dt = -1;
        if(pt1->time != 0x00000000 && pt1->time != 0xFFFFFFFF) {
            dt = pt2->time - pt1->time;
        }

        XY p1,p2;
        p1.u = DEG_TO_RAD * pt1->lon;
        p1.v = DEG_TO_RAD * pt1->lat;
        p2.u = DEG_TO_RAD * pt2->lon;
        p2.v = DEG_TO_RAD * pt2->lat;
        float a2 = 0;

        pt2->delta      = distance(p1,p2,pt1->azimuth,a2);
        pt2->distance   = pt1->distance + pt2->delta;
        pt2->speed      = (dt > 0) ? pt2->delta / dt * 3.6 : 0;

        t2              = pt2->time;
        totalDistance   = pt2->distance;

        pt1 = pt2;
    }

    totalTime = t2 - t1;

    emit sigChanged();
}

