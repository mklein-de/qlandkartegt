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
    quint32 nTrkPts = 0;
    QIODevice * dev = s.device();
    qint64      pos = dev->pos();

    char magic[9];
    s.readRawData(magic,9);

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
                quint32 n;

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
            case CTrack::eTrain:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);

                quint32 nTrkPts1 = 0;

                s1 >> nTrkPts1;
                if(nTrkPts1 != nTrkPts) {
                    QMessageBox::warning(0, QObject::tr("Corrupt track ..."), QObject::tr("Number of trackpoints is not equal the number of training data trackpoints."), QMessageBox::Ignore,QMessageBox::Ignore);
                    break;
                }

                QList<CTrack::pt_t>::iterator pt1 = track.track.begin();
                while (pt1 != track.track.end()) {
                    s1 >> pt1->heartReateBpm;
                    s1 >> pt1->cadenceRpm;
                    pt1++;
                }

                track.setTraineeData();
                break;
            }
            case CTrack::eTrkExt1:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                quint32 nTrkPts1 = 0;

                s1 >> nTrkPts1;
                if(nTrkPts1 != nTrkPts) {
                    QMessageBox::warning(0, QObject::tr("Corrupt track ..."), QObject::tr("Number of trackpoints is not equal the number of extended data trackpoints."), QMessageBox::Ignore,QMessageBox::Ignore);
                    break;
                }

                QList<CTrack::pt_t>::iterator pt1 = track.track.begin();
                while (pt1 != track.track.end()) {
                                 ///< [m]
                    s1 >> pt1->altitude;
                                 ///< [m]
                    s1 >> pt1->height;
                                 ///< [m/s]
                    s1 >> pt1->velocity;
                                 ///< []
                    s1 >> pt1->heading;
                                 ///< []
                    s1 >> pt1->magnetic;
                                 ///<
                    s1 >> pt1->vdop;
                                 ///<
                    s1 >> pt1->hdop;
                                 ///<
                    s1 >> pt1->pdop;
                    s1 >> pt1->x;///< [m] cartesian gps coordinate
                    s1 >> pt1->y;///< [m] cartesian gps coordinate
                    s1 >> pt1->z;///< [m] cartesian gps coordinate
                                 ///< [m/s] velocity
                    s1 >> pt1->vx;
                                 ///< [m/s] velocity
                    s1 >> pt1->vy;
                                 ///< [m/s] velocity
                    s1 >> pt1->vz;
                    pt1++;
                }

                track.setExt1Data();
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

    QList<CTrack::pt_t>& trkpts = track.getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt = trkpts.begin();

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
    // prepare trainings data
    //---------------------------------------
    if(track.traineeData) {
        trk_head_entry_t entryTrainPts;
        entryTrainPts.type = CTrack::eTrain;
        QDataStream s3(&entryTrainPts.data, QIODevice::WriteOnly);

        trkpt = trkpts.begin();

        s3 << (quint32)trkpts.size();
        while(trkpt != trkpts.end()) {
            s3 << trkpt->heartReateBpm;
            s3 << trkpt->cadenceRpm;
            ++trkpt;
        }

        entries << entryTrainPts;
    }
    //---------------------------------------
    // prepare extended trackpoint data 1
    //---------------------------------------
    if(track.ext1Data) {
        trk_head_entry_t entryTrkExt1;
        entryTrkExt1.type = CTrack::eTrkExt1;
        QDataStream s4(&entryTrkExt1.data, QIODevice::WriteOnly);

        trkpt = trkpts.begin();

        s4 << (quint32)trkpts.size();
        while(trkpt != trkpts.end()) {
                                 ///< [m]
            s4 << trkpt->altitude;
            s4 << trkpt->height; ///< [m]
                                 ///< [m/s]
            s4 << trkpt->velocity;
            s4 << trkpt->heading;///< []
                                 ///< []
            s4 << trkpt->magnetic;
            s4 << trkpt->vdop;   ///<
            s4 << trkpt->hdop;   ///<
            s4 << trkpt->pdop;   ///<
            s4 << trkpt->x;      ///< [m] cartesian gps coordinate
            s4 << trkpt->y;      ///< [m] cartesian gps coordinate
            s4 << trkpt->z;      ///< [m] cartesian gps coordinate
            s4 << trkpt->vx;     ///< [m/s] velocity
            s4 << trkpt->vy;     ///< [m/s] velocity
            s4 << trkpt->vz;     ///< [m/s] velocity
            ++trkpt;
        }

        entries << entryTrkExt1;
    }
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


const QColor CTrack::lineColors[] =
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

const QString CTrack::bulletColors[] =
{

    QString(":/icons/small_bullet_black.png")               // 0
    ,QString(":/icons/small_bullet_darkred.png")            // 1
    ,QString(":/icons/small_bullet_darkgreen.png")          // 2
    ,QString(":/icons/small_bullet_darkyellow.png")         // 3
    ,QString(":/icons/small_bullet_darkblue.png")           // 4
    ,QString(":/icons/small_bullet_darkmagenta.png")        // 5
    ,QString(":/icons/small_bullet_darkcyan.png")           // 6
    ,QString(":/icons/small_bullet_gray.png")               // 7
    ,QString(":/icons/small_bullet_darkgray.png")           // 8
    ,QString(":/icons/small_bullet_red.png")                // 9
    ,QString(":/icons/small_bullet_green.png")              // 10
    ,QString(":/icons/small_bullet_yellow.png")             // 11
    ,QString(":/icons/small_bullet_blue.png")               // 12
    ,QString(":/icons/small_bullet_magenta.png")            // 13
    ,QString(":/icons/small_bullet_cyan.png")               // 14
    ,QString(":/icons/small_bullet_white.png")              // 15
    ,QString("")                                          // 16
};

bool trackpointLessThan(const CTrack::pt_t &p1, const CTrack::pt_t &p2)
{
    return p1.timestamp < p2.timestamp;
}


CTrack::CTrack(QObject * parent)
: QObject(parent)
, timestamp(QDateTime::currentDateTime().toUTC().toTime_t ())
, color(Qt::darkBlue)
, colorIdx(4)
, highlight(false)
, traineeData(false)
, ext1Data(false)
, firstTime(true)
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
    QList<CTrack::pt_t>& trkpts = getTrackPoints();
    QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
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
    color       = lineColors[i];
    bullet      = QPixmap(bulletColors[i]);
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


void CTrack::sortByTimestamp()
{
    qSort(track.begin(), track.end(), trackpointLessThan);
}


#define A 0.22140

void CTrack::rebuild(bool reindex)
{

    double slope    = 0;
    IMap& dem = CMapDB::self().getDEM();
    quint32 t1 = 0, t2 = 0;
    QList<pt_t>::iterator pt1 = track.begin();
    QList<pt_t>::iterator pt2 = track.begin();

    totalTime       = 0;
    totalDistance   = 0;
    totalAscend     = 0;
    totalDescend    = 0;
    avgspeed0       = 0;
    avgspeed1       = 0;

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
    while((pt1 != track.end()) && (pt1->flags & pt_t::eDeleted)) {
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
    pt1->slope      = 0.0;
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
        pt2->slope    = qRound(slope / pt2->delta * 10000)/100.0;
        if (qAbs(pt2->slope )>100)
            pt2->slope = pt1->slope;
        // qDebug() << slope << pt2->delta << pt2->slope;
        if(slope > 0) {
            totalAscend  += slope;
        }
        else {
            totalDescend += slope;
        }
        pt2->ascend     = totalAscend;
        pt2->descend    = totalDescend;
        if(ext1Data && pt2->velocity != WPT_NOFLOAT) {
            pt2->speed  =  pt2->velocity;
            //             pt2->speed  = (dt > 0) ? pt2->delta / dt : 0;
        }
        else {
            pt2->speed  = (dt > 0) ? pt2->delta / dt : 0;
        }
        pt2->dem        = dem.getElevation(pt2->lon * DEG_TO_RAD, pt2->lat * DEG_TO_RAD);

        t2              = pt2->timestamp;
        totalDistance   = pt2->distance;

        avgspeed0       = A * pt2->speed + (1.0 - A) * avgspeed1;
        avgspeed1       = avgspeed0;
        pt2->avgspeed   = avgspeed0;

        pt1 = pt2;
    }

    totalTime = t2 - t1;

    emit sigChanged();

}


void CTrack::setPointOfFocus(int idx)
{
    // reset previous selections
    QList<CTrack::pt_t>& trkpts           = track;
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
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


QDataStream& operator >>(QDataStream& s, CFlags& flag)
{
	quint32 f;
	s >> f;
	flag.setFlags(f);
	flag.setChanged(true);
	return s;
}


QDataStream& operator <<(QDataStream& s, CFlags& flag)
{
	s << flag.flag();
	return s;
}
