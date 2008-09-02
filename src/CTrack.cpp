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
#include "CMapDB.h"

#include <QtGui>

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

QDir CTrack::path(_MKSTR(MAPPATH) "/Track");

struct trk_head_entry_t
{
    trk_head_entry_t() : type(CTrack::eEnd), offset(0) {}
    qint32      type;
    quint32     offset;
    QByteArray  data;
};

QDataStream& operator >>(QDataStream& s, CTrack& track)
{
    QIODevice * dev = s.device();
    qint64      pos = dev->pos();

    char magic[9];
    s.readRawData(magic,9);

    qDebug() << magic;

    if(strncmp(magic,"QLTrk   ",9)) {
        dev->seek(pos);
        return s;
    }

    QList<trk_head_entry_t> entries;

    while(1) {
        trk_head_entry_t entry;
        s >> entry.type >> entry.offset;
        entries << entry;
        if(entry.type == CTrack::eEnd) break;
    }

    QList<trk_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end()) {
        qint64 o = pos + entry->offset;
        dev->seek(o);
        s >> entry->data;

        switch(entry->type) {
            case CTrack::eBase:
            {

                QDataStream s1(&entry->data, QIODevice::ReadOnly);

                s1 >> track._key_;
                s1 >> track.timestamp;
                s1 >> track.name;
                s1 >> track.comment;
                s1 >> track.colorIdx;

                break;
            }

            case CTrack::eTrkPts:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                quint32 n, nTrkPts = 0;

                track.track.clear();
                s1 >> nTrkPts;


                for(n = 0; n < nTrkPts; ++n) {
                    CTrack::pt_t trkpt;
                    s1 >> trkpt.lon;
                    s1 >> trkpt.lat;
                    s1 >> trkpt.ele;
                    s1 >> trkpt.timestamp;
                    s1 >> trkpt.flags;

                    track << trkpt;
                }
                break;
            }

            default:;
        }

        ++entry;
    }

    return s;
}


QDataStream& operator <<(QDataStream& s, CTrack& track)
{
    QList<trk_head_entry_t> entries;

    //---------------------------------------
    // prepare base data
    //---------------------------------------
    trk_head_entry_t entryBase;
    entryBase.type = CTrack::eBase;
    QDataStream s1(&entryBase.data, QIODevice::WriteOnly);

    s1 << track.key();
    s1 << track.timestamp;
    s1 << track.name;
    s1 << track.comment;
    s1 << track.colorIdx;

    entries << entryBase;

    //---------------------------------------
    // prepare trackpoint data
    //---------------------------------------
    trk_head_entry_t entryTrkPts;
    entryTrkPts.type = CTrack::eTrkPts;
    QDataStream s2(&entryTrkPts.data, QIODevice::WriteOnly);

    QVector<CTrack::pt_t>& trkpts = track.getTrackPoints();
    QVector<CTrack::pt_t>::iterator trkpt = trkpts.begin();

    s2 << (quint32)trkpts.size();

    while(trkpt != trkpts.end()) {
        s2 << trkpt->lon;
        s2 << trkpt->lat;
        s2 << trkpt->ele;
        s2 << trkpt->timestamp;
        s2 << trkpt->flags;
        ++trkpt;
    }

    entries << entryTrkPts;

    //---------------------------------------
    // prepare terminator
    //---------------------------------------
    trk_head_entry_t entryEnd;
    entryEnd.type = CTrack::eEnd;
    entries << entryEnd;

    //---------------------------------------
    //---------------------------------------
    // now start to actually write data;
    //---------------------------------------
    //---------------------------------------
    // write magic key
    s.writeRawData("QLTrk   ",9);

    // calculate offset table
    quint32 offset = entries.count() * 8 + 9;

    QList<trk_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end()) {
        entry->offset = offset;
        offset += entry->data.size() + sizeof(quint32);
        ++entry;
    }

    // write offset table
    entry = entries.begin();
    while(entry != entries.end()) {
        s << entry->type << entry->offset;
        ++entry;
    }

    // write entry data
    entry = entries.begin();
    while(entry != entries.end()) {
        s << entry->data;
        ++entry;
    }

    return s;
}


void operator >>(QFile& f, CTrack& track)
{
    f.open(QIODevice::ReadOnly);
    QDataStream s(&f);
    s >> track;
    f.close();
}


void operator <<(QFile& f, CTrack& track)
{
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
    s << track;
    f.close();
}


const QColor CTrack::colors[] =
{
    Qt::black                    // 0
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

QRectF CTrack::getBoundingRectF()
{

    double north =  -90.0;
    double south =  +90.0;
    double west  = +180.0;
    double east  = -180.0;

    //CTrack * track = tracks[key];
    QVector<CTrack::pt_t>& trkpts = getTrackPoints();
    QVector<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()) {
        if(!(trkpt->flags & CTrack::pt_t::eDeleted)) {
            if(trkpt->lon < west)  west  = trkpt->lon;
            if(trkpt->lon > east)  east  = trkpt->lon;
            if(trkpt->lat < south) south = trkpt->lat;
            if(trkpt->lat > north) north = trkpt->lat;
        }
        ++trkpt;
    }

    return QRectF(QPointF(west,north),QPointF(east,south));
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

CTrack& CTrack::operator+=(const CTrack& trk)
{
    track += trk.track;
    rebuild(true);
    return *this;
}

void CTrack::rebuild(bool reindex)
{
    double slope    = 0;
    IMap& dem = CMapDB::self().getDEM();
    quint32 t1 = 0, t2 = 0;
    QVector<pt_t>::iterator pt1 = track.begin();
    QVector<pt_t>::iterator pt2 = track.begin();

    totalTime       = 0;
    totalDistance   = 0;
    totalAscend     = 0;
    totalDescend    = 0;


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
        pt1->ascend     = totalAscend;
        pt1->descend    = totalDescend;
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
    pt1->ascend     = totalAscend;
    pt1->descend    = totalDescend;
    pt1->dem        = dem.getElevation(pt1->lon * DEG_TO_RAD, pt1->lat * DEG_TO_RAD);
    t1              = pt1->timestamp;

    // process track
    while(++pt2 != track.end()) {

        // reset deleted points
        if(pt2->flags & pt_t::eDeleted) {
            pt2->azimuth    = WPT_NOFLOAT;
            pt2->delta      = 0;
            pt2->speed      = -1;
            pt2->distance   = 0;
            pt1->ascend     = 0;
            pt1->descend    = 0;
            continue;
        }

        int dt = -1;
        if(pt1->timestamp != 0x00000000 && pt1->timestamp != 0xFFFFFFFF) {
            dt = pt2->timestamp - pt1->timestamp;
        }

        XY p1,p2;
        p1.u = DEG_TO_RAD * pt1->lon;
        p1.v = DEG_TO_RAD * pt1->lat;
        p2.u = DEG_TO_RAD * pt2->lon;
        p2.v = DEG_TO_RAD * pt2->lat;
        double a2 = 0;

        pt2->delta      = distance(p1,p2,pt1->azimuth,a2);
        pt2->distance   = pt1->distance + pt2->delta;
        slope           = pt2->ele - pt1->ele;
        if(slope > 0){
            totalAscend  += slope;
        }
        else{
            totalDescend += slope;
        }
        pt2->ascend     = totalAscend;
        pt2->descend    = totalDescend;
        pt2->speed      = (dt > 0) ? pt2->delta / dt : 0;
        pt2->dem        = dem.getElevation(pt2->lon * DEG_TO_RAD, pt2->lat * DEG_TO_RAD);

        t2              = pt2->timestamp;
        totalDistance   = pt2->distance;

        pt1 = pt2;

    }

    totalTime = t2 - t1;

    emit sigChanged();
}


void CTrack::setPointOfFocus(int idx)
{
    // reset previous selections
    QVector<CTrack::pt_t>& trkpts           = track;
    QVector<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end()) {
        trkpt->flags &= ~CTrack::pt_t::eFocus;
        ++trkpt;
    }
    if(idx < track.count()) {
        trkpts[idx].flags |= CTrack::pt_t::eFocus;
        trkpts[idx].flags |= CTrack::pt_t::eSelected;
    }
    emit sigChanged();
}
