/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/

#include "CTrack.h"
#include "GeoMath.h"
#include "CMapDB.h"
#include "CResources.h"
#include "IUnit.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CWptDB.h"
#include "CTrackDB.h"
#include "CSettings.h"

#include <QtGui>
#include <QtNetwork/QHttp>
#include <QtNetwork/QNetworkProxy>

#include <tzdata.h>

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

    if(strncmp(magic,"QLTrk   ",9))
    {
        dev->seek(pos);
        return s;
    }

    QList<trk_head_entry_t> entries;

    while(1)
    {
        trk_head_entry_t entry;
        s >> entry.type >> entry.offset;
        entries << entry;
        if(entry.type == CTrack::eEnd) break;
    }

    QList<trk_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end())
    {
        qint64 o = pos + entry->offset;
        dev->seek(o);
        s >> entry->data;

        switch(entry->type)
        {
            case CTrack::eBase:
            {

                QString key;

                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);

                s1 >> key;
                s1 >> track.timestamp;
                s1 >> track.name;
                s1 >> track.comment;
                s1 >> track.colorIdx;
                s1 >> track.parentWpt;
                if(!s1.atEnd())
                {
                    s1 >> track.doScaleWpt2Track;
                }
                if(!s1.atEnd())
                {
                    s1 >> track.cntMedianFilterApplied;
                }
                if(!s1.atEnd())
                {
                    s1 >> track.useMultiColor;
                }
                if(!s1.atEnd())
                {
                    s1 >> track.idMultiColor;
                }

                track.setColor(track.colorIdx);
                track.setKey(key);

                break;
            }

            case CTrack::eTrkPts:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);
                quint32 n;

                track.track.clear();
                s1 >> nTrkPts;

                for(n = 0; n < nTrkPts; ++n)
                {
                    CTrack::pt_t trkpt;
                    s1 >> trkpt.lon;
                    s1 >> trkpt.lat;
                    s1 >> trkpt.ele;
                    s1 >> trkpt.timestamp;
                    s1 >> trkpt.flags;

                    trkpt._lon = trkpt.lon;
                    trkpt._lat = trkpt.lat;
                    trkpt._ele = trkpt.ele;
                    trkpt._timestamp = trkpt.timestamp;
                    trkpt._timestamp_msec = trkpt.timestamp_msec;

                    track << trkpt;
                }
                break;
            }

            case CTrack::eTrkPts2:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);

                quint32 nTrkPts1 = 0;

                s1 >> nTrkPts1;
                if(nTrkPts1 != nTrkPts)
                {
                    QMessageBox::warning(0, QObject::tr("Corrupt track ..."), QObject::tr("Number of trackpoints is not equal the number of training data trackpoints."), QMessageBox::Ignore,QMessageBox::Ignore);
                    break;
                }

                QList<CTrack::pt_t>::iterator pt1 = track.track.begin();
                while (pt1 != track.track.end())
                {
                    quint32 dummy;
                    s1 >> pt1->timestamp_msec;
                    s1 >> dummy;
                    s1 >> dummy;
                    s1 >> dummy;
                    s1 >> dummy;
                    s1 >> dummy;

                    pt1++;
                }
                break;
            }

            case CTrack::eTrain:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);

                quint32 nTrkPts1 = 0;

                s1 >> nTrkPts1;
                if(nTrkPts1 != nTrkPts)
                {
                    QMessageBox::warning(0, QObject::tr("Corrupt track ..."), QObject::tr("Number of trackpoints is not equal the number of training data trackpoints."), QMessageBox::Ignore,QMessageBox::Ignore);
                    break;
                }

                QList<CTrack::pt_t>::iterator pt1 = track.track.begin();
                while (pt1 != track.track.end())
                {
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
                s1.setVersion(QDataStream::Qt_4_5);
                quint32 nTrkPts1 = 0;

                s1 >> nTrkPts1;
                if(nTrkPts1 != nTrkPts)
                {
                    QMessageBox::warning(0, QObject::tr("Corrupt track ..."), QObject::tr("Number of trackpoints is not equal the number of extended data trackpoints."), QMessageBox::Ignore,QMessageBox::Ignore);
                    break;
                }

                QList<CTrack::pt_t>::iterator pt1 = track.track.begin();
                while (pt1 != track.track.end())
                {
                                 ///< [m]
                    s1 >> pt1->altitude;
                                 ///< [m]
                    s1 >> pt1->height;
                                 ///< [m/s]
                    s1 >> pt1->velocity;
                                 ///< [deg]
                    s1 >> pt1->heading;
                                 ///< [deg]
                    s1 >> pt1->magnetic;
                    s1 >> pt1->vdop;
                    s1 >> pt1->hdop;
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
#ifdef GPX_EXTENSIONS
            case CTrack::eTrkGpxExt:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);
                quint32 nTrkPts1 = 0;

                s1 >> nTrkPts1;
                if(nTrkPts1 != nTrkPts)
                {
                    QMessageBox::warning(0, QObject::tr("Corrupt track ..."), QObject::tr("Number of trackpoints is not equal the number of extended data trackpoints."), QMessageBox::Ignore,QMessageBox::Ignore);
                    break;
                }

                track.tr_ext.set.clear();
                s1 >> track.tr_ext.set;

                QList<CTrack::pt_t>::iterator pt1 = track.track.begin();
                while (pt1 != track.track.end())
                {
                    pt1->gpx_exts.values.clear();
                    s1 >> pt1->gpx_exts.values;
                    pt1++;
                }
                break;
            }
#endif
            case CTrack::eTrkShdw:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);
                quint32 n;

                quint32 nTrkPts1 = 0;

                s1 >> nTrkPts1;
                if(nTrkPts1 != nTrkPts)
                {
                    QMessageBox::warning(0, QObject::tr("Corrupt track ..."), QObject::tr("Number of trackpoints is not equal the number of shadow data trackpoints."), QMessageBox::Ignore,QMessageBox::Ignore);
                    break;
                }

                for(n = 0; n < nTrkPts; ++n)
                {
                    CTrack::pt_t& trkpt = track.track[n];
                    s1 >> trkpt._lon;
                    s1 >> trkpt._lat;
                    s1 >> trkpt._ele;
                }
                break;
            }

            case CTrack::eTrkShdw2:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);
                quint32 n;

                quint32 nTrkPts1 = 0;

                s1 >> nTrkPts1;
                if(nTrkPts1 != nTrkPts)
                {
                    QMessageBox::warning(0, QObject::tr("Corrupt track ..."), QObject::tr("Number of trackpoints is not equal the number of shadow data trackpoints."), QMessageBox::Ignore,QMessageBox::Ignore);
                    break;
                }

                for(n = 0; n < nTrkPts; ++n)
                {
                    quint32 dummy;
                    CTrack::pt_t& trkpt = track.track[n];
                    s1 >> trkpt._timestamp;
                    s1 >> trkpt._timestamp_msec;
                    s1 >> dummy;
                    s1 >> dummy;
                    s1 >> dummy;
                    s1 >> dummy;
                    s1 >> dummy;
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
    s1.setVersion(QDataStream::Qt_4_5);

    s1 << track.getKey();
    s1 << track.timestamp;
    s1 << track.name;
    s1 << track.comment;
    s1 << track.colorIdx;
    s1 << track.getParentWpt();
    s1 << track.doScaleWpt2Track;
    s1 << track.cntMedianFilterApplied;
    s1 << track.useMultiColor;
    s1 << track.idMultiColor;

    entries << entryBase;

    //---------------------------------------
    // prepare trackpoint data
    //---------------------------------------
    trk_head_entry_t entryTrkPts;
    entryTrkPts.type = CTrack::eTrkPts;
    QDataStream s2(&entryTrkPts.data, QIODevice::WriteOnly);
    s2.setVersion(QDataStream::Qt_4_5);

    QList<CTrack::pt_t>& trkpts = track.getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt = trkpts.begin();

    s2 << (quint32)trkpts.size();
    while(trkpt != trkpts.end())
    {
        s2 << trkpt->lon;
        s2 << trkpt->lat;
        s2 << trkpt->ele;
        s2 << trkpt->timestamp;
        s2 << trkpt->flags;
        ++trkpt;
    }

    entries << entryTrkPts;

    //---------------------------------------
    // prepare trackpoint data 2
    //---------------------------------------
    trk_head_entry_t entryTrkPts2;
    entryTrkPts2.type = CTrack::eTrkPts2;
    QDataStream s8(&entryTrkPts2.data, QIODevice::WriteOnly);
    s8.setVersion(QDataStream::Qt_4_5);

    trkpt = trkpts.begin();

    s8 << (quint32)trkpts.size();
    while(trkpt != trkpts.end())
    {
        s8 << trkpt->timestamp_msec;
        s8 << quint32(0);
        s8 << quint32(0);
        s8 << quint32(0);
        s8 << quint32(0);
        s8 << quint32(0);
        ++trkpt;
    }

    entries << entryTrkPts2;


    //---------------------------------------
    // prepare trainings data
    //---------------------------------------
    if(track.traineeData)
    {
        trk_head_entry_t entryTrainPts;
        entryTrainPts.type = CTrack::eTrain;
        QDataStream s3(&entryTrainPts.data, QIODevice::WriteOnly);
        s3.setVersion(QDataStream::Qt_4_5);

        trkpt = trkpts.begin();

        s3 << (quint32)trkpts.size();
        while(trkpt != trkpts.end())
        {
            s3 << trkpt->heartReateBpm;
            s3 << trkpt->cadenceRpm;
            ++trkpt;
        }

        entries << entryTrainPts;
    }
    //---------------------------------------
    // prepare extended trackpoint data 1
    //---------------------------------------
    if(track.ext1Data)
    {
        trk_head_entry_t entryTrkExt1;
        entryTrkExt1.type = CTrack::eTrkExt1;
        QDataStream s4(&entryTrkExt1.data, QIODevice::WriteOnly);
        s4.setVersion(QDataStream::Qt_4_5);

        trkpt = trkpts.begin();

        s4 << (quint32)trkpts.size();
        while(trkpt != trkpts.end())
        {
                                 ///< [m]
            s4 << trkpt->altitude;
            s4 << trkpt->height; ///< [m]
                                 ///< [m/s]
            s4 << trkpt->velocity;
            s4 << trkpt->heading;///< [deg]
                                 ///< [deg]
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
#ifdef GPX_EXTENSIONS
    //---------------------------------------
    // prepare extended gpx trackpoint data
    //---------------------------------------
    if(track.tr_ext.set.size())
    {
        trk_head_entry_t entryTrkGpxExt;
        entryTrkGpxExt.type = CTrack::eTrkGpxExt;
        QDataStream s5(&entryTrkGpxExt.data, QIODevice::WriteOnly);
        s5.setVersion(QDataStream::Qt_4_5);

        trkpt = trkpts.begin();

        s5 << (quint32)trkpts.size();
        s5 << track.tr_ext.set;

        while(trkpt != trkpts.end())
        {
            s5 << trkpt->gpx_exts.values;
            ++trkpt;
        }

        entries << entryTrkGpxExt;
    }
#endif

    //---------------------------------------
    // prepare track shadow data
    //---------------------------------------
    trk_head_entry_t entryShdwPts;
    entryShdwPts.type = CTrack::eTrkShdw;
    QDataStream s6(&entryShdwPts.data, QIODevice::WriteOnly);
    s6.setVersion(QDataStream::Qt_4_5);

    trkpt = trkpts.begin();

    s6 << (quint32)trkpts.size();
    while(trkpt != trkpts.end())
    {
        s6 << trkpt->_lon;
        s6 << trkpt->_lat;
        s6 << trkpt->_ele;
        ++trkpt;
    }

    entries << entryShdwPts;

    //---------------------------------------
    // prepare track shadow data 2
    //---------------------------------------
    quint32 dummy = 0;
    trk_head_entry_t entryShdwPts2;
    entryShdwPts2.type = CTrack::eTrkShdw2;
    QDataStream s7(&entryShdwPts2.data, QIODevice::WriteOnly);
    s7.setVersion(QDataStream::Qt_4_5);

    trkpt = trkpts.begin();

    s7 << (quint32)trkpts.size();
    while(trkpt != trkpts.end())
    {
        s7 << trkpt->_timestamp;
        s7 << trkpt->_timestamp_msec;
        s7 << dummy;
        s7 << dummy;
        s7 << dummy;
        s7 << dummy;
        s7 << dummy;
        ++trkpt;
    }

    entries << entryShdwPts2;

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
    while(entry != entries.end())
    {
        entry->offset = offset;
        offset += entry->data.size() + sizeof(quint32);
        ++entry;
    }

    // write offset table
    entry = entries.begin();
    while(entry != entries.end())
    {
        s << entry->type << entry->offset;
        ++entry;
    }

    // write entry data
    entry = entries.begin();
    while(entry != entries.end())
    {
        s << entry->data;
        ++entry;
    }

    return s;
}


void operator >>(QFile& f, CTrack& track)
{
    f.open(QIODevice::ReadOnly);
    QDataStream s(&f);
    s.setVersion(QDataStream::Qt_4_5);
    s >> track;
    f.close();
}


void operator <<(QFile& f, CTrack& track)
{
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
    s.setVersion(QDataStream::Qt_4_5);
    s << track;
    f.close();
}


#define DEFAULT_COLOR 4

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

                                 // 0
    QString(":/icons/small_bullet_black.png")
                                 // 1
    ,QString(":/icons/small_bullet_darkred.png")
                                 // 2
    ,QString(":/icons/small_bullet_darkgreen.png")
                                 // 3
    ,QString(":/icons/small_bullet_darkyellow.png")
                                 // 4
    ,QString(":/icons/small_bullet_darkblue.png")
                                 // 5
    ,QString(":/icons/small_bullet_darkmagenta.png")
                                 // 6
    ,QString(":/icons/small_bullet_darkcyan.png")
                                 // 7
    ,QString(":/icons/small_bullet_gray.png")
                                 // 8
    ,QString(":/icons/small_bullet_darkgray.png")
                                 // 9
    ,QString(":/icons/small_bullet_red.png")
                                 // 10
    ,QString(":/icons/small_bullet_green.png")
                                 // 11
    ,QString(":/icons/small_bullet_yellow.png")
                                 // 12
    ,QString(":/icons/small_bullet_blue.png")
                                 // 13
    ,QString(":/icons/small_bullet_magenta.png")
                                 // 14
    ,QString(":/icons/small_bullet_cyan.png")
                                 // 15
    ,QString(":/icons/small_bullet_white.png")
    ,QString("")                 // 16
};

bool trackpointLessThan(const CTrack::pt_t &p1, const CTrack::pt_t &p2)
{
    return p1.timestamp < p2.timestamp;
}


static bool qSortWptLessDistance(CTrack::wpt_t& p1, CTrack::wpt_t& p2)
{
    return p1.trkpt.distance < p2.trkpt.distance;
}


CTrack::CTrack(QObject * parent)
: IItem(parent)
, color(Qt::darkBlue)
, bullet(":/icons/small_bullet_darkblue.png")
, colorIdx(DEFAULT_COLOR)
, highlight(false)
, traineeData(false)
, ext1Data(false)
, firstTime(true)
, m_hide(false)
, doScaleWpt2Track(Qt::PartiallyChecked )
, visiblePointCount(0)
, cntMedianFilterApplied(0)
, replaceOrigData(true)
, stateSelect(e1stSel)
, useMultiColor(false)
, idMultiColor(eMultiColorEle)
{
    ref = 1;

    setColor(DEFAULT_COLOR);

    networkAccessManager = new QNetworkAccessManager(this);
    networkAccessManager->setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy));
    connect(networkAccessManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),this, SLOT(slotProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
    connect(networkAccessManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(slotRequestFinished(QNetworkReply*)));

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotScaleWpt2Track()));
    connect(&CWptDB::self(), SIGNAL(sigModified(QString)), this, SLOT(slotScaleWpt2Track()));
}


CTrack::~CTrack()
{

}

void CTrack::getMultiColor(bool& on, int& id, QList<multi_color_item_t>& items)
{
    items << multi_color_item_t(tr("solid"), eMultiColorNone);
    items << multi_color_item_t(tr("slope"), eMultiColorSlope);
    items << multi_color_item_t(tr("elevation"), eMultiColorEle);

    on = useMultiColor;
    id = idMultiColor;
}

void CTrack::setMultiColor(bool on, int id)
{
     useMultiColor = on;
     idMultiColor  = id;

     rebuild(false);
}

void CTrack::setHighlight(bool yes)
{
    highlight = yes;
    if(yes)
    {
        slotScaleWpt2Track();
    }
}


void CTrack::replaceElevationByLocal(bool replaceOrignalData)
{
    IMap& map = CMapDB::self().getDEM();

    QList<CTrack::pt_t>::iterator trkpt, end;
    setupIterators(trkpt, end);
    while(trkpt != end)
    {
        trkpt->ele = map.getElevation(trkpt->lon * DEG_TO_RAD, trkpt->lat * DEG_TO_RAD);
        if(replaceOrignalData)
        {
            trkpt->_ele   = trkpt->ele;
        }
        trkpt++;
    }
    rebuild(false);
    emit sigChanged();
}


void CTrack::replaceElevationByRemote(bool replaceOrignalData)
{
    SETTINGS;
    QString username = cfg.value("geonames/username","demo").toString();

    replaceOrigData = replaceOrignalData;

    QList<CTrack::pt_t>::iterator trkpt, end;
    setupIterators(trkpt, end);

    reply2idx.clear();

    while(trkpt != end)
    {
        int s = (end - trkpt) > 20 ? 20 : (end - trkpt);
        int idx = trkpt->idx;

        QStringList lats, lngs;
        for(int i=0; i < s; i++)
        {
            lats << QString::number(trkpt->lat,'f', 8);
            lngs << QString::number(trkpt->lon,'f', 8);
            trkpt++;
        }

        QUrl url("http://ws.geonames.org");
        url.setPath("/srtm3");
        url.addQueryItem("lats",lats.join(","));
        url.addQueryItem("lngs",lngs.join(","));
        url.addQueryItem("username",username);

        QNetworkRequest request;
        request.setUrl(url);
        QNetworkReply * reply = networkAccessManager->get(request);

        reply2idx[reply] = idx;

        idx += s;
    }
}


void CTrack::slotProxyAuthenticationRequired(const QNetworkProxy &prox, QAuthenticator *auth)
{
    QString user;
    QString pwd;

    CResources::self().getHttpProxyAuth(user,pwd);

    auth->setUser(user);
    auth->setPassword(pwd);
}


void CTrack::slotRequestFinished(QNetworkReply * reply)
{
    if(reply->error() != QNetworkReply::NoError)
    {
        return;
    }

    QString asw = reply->readAll();
    reply->deleteLater();

    if(asw.isEmpty())
    {
        return;
    }

    QString val;
    QStringList vals = asw.split("\n", QString::SkipEmptyParts);

    if(reply2idx.contains(reply))
    {

        int idx = reply2idx[reply];
        foreach(val, vals)
        {
            if(idx < track.size())
            {
                track[idx].ele    = val.toDouble();
                if(replaceOrigData)
                {
                    track[idx]._ele   = track[idx].ele;
                }
                idx++;
            }
        }

        reply2idx.remove(reply);
        if(reply2idx.isEmpty())
        {
            rebuild(false);
            emit sigChanged();
        }
    }
    else
    {
        qDebug() << "failed to recognize reply";
    }
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
    while(trkpt != trkpts.end())
    {
        if(!(trkpt->flags & CTrack::pt_t::eDeleted))
        {
            if(trkpt->lon < west)  west  = trkpt->lon;
            if(trkpt->lon > east)  east  = trkpt->lon;
            if(trkpt->lat < south) south = trkpt->lat;
            if(trkpt->lat > north) north = trkpt->lat;
        }
        ++trkpt;
    }

    return QRectF(QPointF(west,north),QPointF(east,south));
}


void CTrack::setColor(const QColor& c)
{
    int n;
    int N = sizeof(lineColors)/sizeof(QColor);

    for(n = 0; n < N; n++)
    {
        if(lineColors[n] == c)
        {
            colorIdx    = n;
            color       = lineColors[n];
            bullet      = QPixmap(bulletColors[n]);
            break;
        }
    }

    if(n == N)
    {
        colorIdx    = DEFAULT_COLOR;
        color       = lineColors[DEFAULT_COLOR];
        bullet      = QPixmap(bulletColors[DEFAULT_COLOR]);
    }

    setIcon(color.name());

}


void CTrack::setColor(unsigned i)
{
    if(i>16) i = DEFAULT_COLOR;
    colorIdx    = i;
    color       = lineColors[i];
    bullet      = QPixmap(bulletColors[i]);

    setIcon(color.name());
}


CTrack& CTrack::operator<<(const pt_t& pt)
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


void CTrack::hide(bool ok)
{
    m_hide = ok;
    if(!ok) firstTime = true;
}


#define A 0.22140

void CTrack::rebuild(bool reindex)
{
    bool haveTimezone  = false;
    double slope    = 0;
    IMap& dem = CMapDB::self().getDEM();
    quint32 t1 = 0, t2 = 0;
    QList<pt_t>::iterator pt1 = track.begin();
    QList<pt_t>::iterator pt2 = track.begin();

    QList<pt_t>::iterator pt1Slope;

    visiblePointCount = 0;
    totalTime       = 0;
    totalTimeMoving = 0;
    totalDistance   = 0;
    totalAscend     = 0;
    totalDescend    = 0;
    avgspeed0       = 0;
    avgspeed1       = 0;
    float maxEle    = -WPT_NOFLOAT;
    float minEle    =  WPT_NOFLOAT;
    float maxSpeed  = -WPT_NOFLOAT;
    float minSpeed  =  WPT_NOFLOAT;

    // reindex track if desired
    if(reindex)
    {
        quint32 idx = 0;
        while(pt1 != track.end())
        {
            pt1->idx = idx++;
            pt1->flags.setChanged(true);
            ++pt1;
        }
        pt1 = track.begin();
    }

    // skip leading deleted points
    while((pt1 != track.end()) && (pt1->flags & pt_t::eDeleted))
    {
        pt1->azimuth    = 0;
        pt1->delta      = 0;
        pt1->speed      = -1;
        pt1->distance   = 0;
        pt1->ascend     = totalAscend;
        pt1->descend    = totalDescend;
        ++pt1; ++pt2;
    }

    // no points at all?
    if(pt1 == track.end())
    {
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
    pt1->slope2     = 0.0;
    t1              = pt1->timestamp;
    t2              = t1;        //for the case that the track has only 1 point

    pt1Slope = pt1;

    ++visiblePointCount;

    if(pt1->ele   > maxEle)   {maxEle   = pt1->ele;   ptMaxEle   = *pt1;}
    if(pt1->ele   < minEle)   {minEle   = pt1->ele;   ptMinEle   = *pt1;}
    if(pt1->speed > maxSpeed) {maxSpeed = pt1->speed; ptMaxSpeed = *pt1;}
    if(pt1->speed < minSpeed) {minSpeed = pt1->speed; ptMinSpeed = *pt1;}

    // process track
    while(++pt2 != track.end())
    {

        // reset deleted points
        if(pt2->flags & pt_t::eDeleted)
        {
            pt2->azimuth    = WPT_NOFLOAT;
            pt2->delta      = 0;
            pt2->speed      = -1;
            pt2->distance   = 0;
            pt2->ascend     = 0;
            pt2->descend    = 0;
            continue;
        }

        ++visiblePointCount;

        if(!haveTimezone)
        {
            timezone        = GPS_Timezone(pt1->lon, pt1->lat);
            haveTimezone    = timezone != "UTC";
        }

        double dt = -1;
        if(pt1->timestamp != 0x00000000 && pt1->timestamp != 0xFFFFFFFF)
        {
            dt = pt2->timestamp + double(pt2->timestamp_msec)/1000 - pt1->timestamp - double(pt1->timestamp_msec)/1000;
        }

        projXY p1,p2;
        p1.u = DEG_TO_RAD * pt1->lon;
        p1.v = DEG_TO_RAD * pt1->lat;
        p2.u = DEG_TO_RAD * pt2->lon;
        p2.v = DEG_TO_RAD * pt2->lat;
        double a2 = 0;

        pt2->delta      = distance(p1,p2,pt1->azimuth,a2);
        pt2->distance   = pt1->distance + pt2->delta;
        if ((pt1->ele != WPT_NOFLOAT) && (pt2->ele != WPT_NOFLOAT))
        {
            slope = pt2->ele - pt1->ele;
        }
        else
        {
            slope = 0.;
        }

        pt2->slope    = atan(slope / pt2->delta) * 360 / (2*M_PI);
        if (qAbs(pt2->slope )>100)
        {
            pt2->slope = pt1->slope;
        }

        if(slope > 0)
        {
            totalAscend  += slope;
        }
        else
        {
            totalDescend += slope;
        }
        pt2->ascend     = totalAscend;
        pt2->descend    = totalDescend;
        if(ext1Data && pt2->velocity != WPT_NOFLOAT)
        {
            pt2->speed  =  pt2->velocity;
        }
        else
        {
            pt2->speed  = (dt > 0) ? pt2->delta / dt : 0;
        }
        pt2->dem        = dem.getElevation(pt2->lon * DEG_TO_RAD, pt2->lat * DEG_TO_RAD);

        t2              = pt2->timestamp;
        totalDistance   = pt2->distance;

        avgspeed0       = A * pt2->speed + (1.0 - A) * avgspeed1;
        avgspeed1       = avgspeed0;
        pt2->avgspeed   = avgspeed0;

        if(pt2->ele   > maxEle)   {maxEle   = pt2->ele;   ptMaxEle   = *pt2;}
        if(pt2->ele   < minEle)   {minEle   = pt2->ele;   ptMinEle   = *pt2;}
        if(pt2->speed > maxSpeed) {maxSpeed = pt2->speed; ptMaxSpeed = *pt2;}
        if(pt2->speed < minSpeed) {minSpeed = pt2->speed; ptMinSpeed = *pt2;}

        pt2->timeSinceStart = t2 - t1;

        if(dt > 0 && (pt2->delta/dt > 0.2))
        {
            totalTimeMoving += dt;

        }

        if ((pt1Slope->ele != WPT_NOFLOAT) && (pt2->ele != WPT_NOFLOAT))
        {
            if((pt2->distance - pt1Slope->distance) > 100)
            {

            }
        }


        pt1 = pt2;
    }

    totalTime = t2 - t1;

    // calculate slope
    for(int i = 0; i<track.size(); i++)
    {
        pt_t & pt = track[i];
        if((pt.flags & pt_t::eDeleted) || (pt.ele == WPT_NOFLOAT))
        {
            pt.slope2 = WPT_NOFLOAT;
            continue;
        }

        float d1 = pt.distance, e1 = pt.ele;
        int n = i;
        while(n>0)
        {
            pt_t & pt2 = track[n];
            if((pt2.flags & pt_t::eDeleted) || (pt2.ele == WPT_NOFLOAT))
            {
                n--;
                continue;
            }

            if(pt.distance - pt2.distance >= 25)
            {
                d1 = pt2.distance;
                e1 = pt2.ele;
                break;
            }
            n--;
        }

        float d2 = pt.distance, e2 = pt.ele;
        n = i;
        while(n < track.size())
        {
            pt_t & pt2 = track[n];
            if((pt2.flags & pt_t::eDeleted) || (pt2.ele == WPT_NOFLOAT))
            {
                n++;
                continue;
            }

            if(pt2.distance - pt.distance >= 25)
            {
                d2 = pt2.distance;
                e2 = pt2.ele;
                break;
            }
            n++;
        }

        float a  = atan((e2 - e1)/(d2 - d1));
        pt.slope2 = fabs(a * 360.0/(2 * M_PI));
        qDebug() << d1 << d2 << e1 << e2 << pt.slope2;

    }


    rebuildColorMap();

    emit sigChanged();
}


void CTrack::rebuildColorMap()
{
    if(!useMultiColor)
    {
        return;
    }
    switch(idMultiColor)
    {
        case eMultiColorSlope:
            rebuildColorMapSlope();
            break;
        case eMultiColorEle:
            rebuildColorMapElevation();
            break;
        default:
            rebuildColorMapDefault();
    }
}

void CTrack::rebuildColorMapElevation()
{
    QColor colors[240];
    for(int i = 0; i<240; i++)
    {
        colors[i].setHsv(240 - i, 255, 255, 255);
    }

    int min  = ptMinEle.ele;
    int max  = ptMaxEle.ele;
    float itvl = float(max - min) / 240;

    for(int i = 0; i < track.size(); i++)
    {
        pt_t & pt = track[i];
        if((pt.flags & pt_t::eDeleted) || (pt.ele == WPT_NOFLOAT))
        {
            continue;
        }
        pt.color = colors[(int)floor((pt.ele - min)/itvl)];
    }
}

void CTrack::rebuildColorMapSlope()
{
    QColor colors[125];
    for(int i = 0; i<125; i++)
    {
        colors[i].setHsv(125 - i, 255, 255, 255);
    }
    for(int i = 0; i < track.size(); i++)
    {
        pt_t & pt = track[i];
        if((pt.flags & pt_t::eDeleted) || (pt.slope2 == WPT_NOFLOAT))
        {
            continue;
        }

        if(pt.slope2 > 25)
        {
            pt.color = colors[124];
        }
        else
        {
            pt.color = colors[(int)floor(125.0/25.0 * pt.slope2)];
        }
    }

}

void CTrack::rebuildColorMapDefault()
{
    for(int i = 0; i < track.size(); i++)
    {
        pt_t & pt = track[i];
        if((pt.flags & pt_t::eDeleted) || (pt.ele == WPT_NOFLOAT))
        {
            continue;
        }
        pt.color = color;
    }
}


void CTrack::setPointOfFocus(int idx, type_select_e typeSelect, bool moveMap)
{
    // reset previous selections
    QList<CTrack::pt_t>::iterator trkpt   = track.begin();

    if(typeSelect == e3Way)
    {
        switch(stateSelect)
        {
            case eNoSel:
                while(trkpt != track.end())
                {
                    trkpt->flags &= ~CTrack::pt_t::eFocus;
                    trkpt->flags &= ~CTrack::pt_t::eSelected;
                    ++trkpt;
                }
                stateSelect = e1stSel;
                break;

            case e1stSel:
                while(trkpt != track.end())
                {
                    trkpt->flags &= ~CTrack::pt_t::eFocus;
                    trkpt->flags &= ~CTrack::pt_t::eSelected;
                    ++trkpt;
                }

                if(idx < track.count())
                {
                    track[idx].flags |= CTrack::pt_t::eFocus;
                    track[idx].flags |= CTrack::pt_t::eSelected;
                }
                stateSelect = e2ndSel;
                break;

            case e2ndSel:
                while(trkpt != track.end())
                {
                    if(trkpt->flags & CTrack::pt_t::eFocus)
                    {
                        break;
                    }
                    ++trkpt;
                }

                int inc = (trkpt->idx < idx) ? +1 : -1;
                while(trkpt != track.end())
                {
                    if(!(trkpt->flags & CTrack::pt_t::eDeleted))
                    {
                        trkpt->flags |= CTrack::pt_t::eSelected;
                    }

                    if(trkpt->idx == idx)
                    {
                        trkpt->flags |= CTrack::pt_t::eFocus;
                        break;
                    }
                    trkpt += inc;
                }
                stateSelect = eNoSel;
                break;
        }
    }
    else
    {
        // erase all flags
        while(trkpt != track.end())
        {
            trkpt->flags &= ~CTrack::pt_t::eFocus;
            if(typeSelect == eErase)
            {
                trkpt->flags &= ~CTrack::pt_t::eSelected;
            }
            ++trkpt;
        }

        // set flags for selected point
        if(idx < track.count())
        {
            track[idx].flags |= CTrack::pt_t::eFocus;
            track[idx].flags |= CTrack::pt_t::eSelected;
        }
    }

    // move map to point under focus
    if(moveMap && idx < track.count())
    {
        theMainWindow->getCanvas()->move(track[idx].lon, track[idx].lat);
    }
    emit sigChanged();
}

void CTrack::getPointOfFocus(QList<CTrack::pt_t>& points)
{
    foreach(const pt_t& trkpt, track)
    {
        if(trkpt.flags & pt_t::eFocus)
        {
            points << trkpt;
        }
    }
}

void CTrack::setupIterators(QList<pt_t>::iterator& begin, QList<pt_t>::iterator& end)
{
    QList<pt_t>& trkpts     = getTrackPoints();
    QList<pt_t>::iterator i = trkpts.begin();

    begin = trkpts.begin();
    end   = trkpts.end();

    int cnt = 0;

    while(i != trkpts.end())
    {
        if(i->flags & pt_t::eFocus)
        {
            cnt++;
            if(begin == track.begin())
            {
                begin = i;
            }
            else
            {
                end = i;
            }
        }
        i++;
    }

    if(cnt < 2)
    {
        begin = trkpts.begin();
        end   = trkpts.end();
    }
}


QDateTime CTrack::getStartTimestamp()
{
    QList<CTrack::pt_t>& trkpts           = track;
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        if(trkpt->flags & pt_t::eDeleted)
        {
            ++trkpt;
            continue;
        }

        QDateTime time = QDateTime::fromTime_t(trkpt->timestamp);
        if(!timezone.isEmpty())
        {
            time = TimeStamp(trkpt->timestamp).toZone(timezone).toDateTime();
        }
        return time;
    }
    return QDateTime();
}


QDateTime CTrack::getEndTimestamp()
{
    QList<CTrack::pt_t>& trkpts           = track;
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.end() - 1;
    while(trkpt > trkpts.begin())
    {
        if(trkpt->flags & pt_t::eDeleted)
        {
            --trkpt;
            continue;
        }
        QDateTime time = QDateTime::fromTime_t(trkpt->timestamp);
        if(!timezone.isEmpty())
        {
            time = TimeStamp(trkpt->timestamp).toZone(timezone).toDateTime();
        }
        return time;
    }
    return QDateTime();
}


float CTrack::getStartElevation()
{
    QList<CTrack::pt_t>& trkpts           = track;
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        if(trkpt->flags & pt_t::eDeleted)
        {
            ++trkpt;
            continue;
        }
        return trkpt->ele;
    }
    return WPT_NOFLOAT;
}


float CTrack::getEndElevation()
{
    QList<CTrack::pt_t>& trkpts           = track;
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.end() - 1;
    while(trkpt > trkpts.begin())
    {
        if(trkpt->flags & pt_t::eDeleted)
        {
            --trkpt;
            continue;
        }
        return trkpt->ele;
    }
    return WPT_NOFLOAT;
}


/// get a summary of item's data to display on screen or in the toolview
QString CTrack::getInfo()
{
    QString val1, unit1, val2, unit2;
    QString str     = getName();
    double distance = getTotalDistance();

    // ---------------------------
    IUnit::self().meter2distance(getTotalDistance(), val1, unit1);
    str += tr("\nlength: %1 %2").arg(val1).arg(unit1);
    str += tr(", points: %1 (%2)").arg(visiblePointCount).arg(getTrackPoints().count());

    // ---------------------------
    quint32 ttime = getTotalTime();
    quint32 days  = ttime / 86400;

    QTime time;
    time = time.addSecs(ttime);
    if(days)
    {
        str += tr("\ntime: %1:").arg(days) + time.toString("HH:mm:ss");
    }
    else
    {
        str += tr("\ntime: ") + time.toString("HH:mm:ss");
    }

    IUnit::self().meter2speed(distance / ttime, val1, unit1);
    str += tr(", speed: %1 %2").arg(val1).arg(unit1);

    // ---------------------------
    ttime = getTotalTimeMoving();
    days  = ttime / 86400;

    time = QTime();
    time = time.addSecs(ttime);
    if(days)
    {
        str += tr("\nmoving: %1:").arg(days) + time.toString("HH:mm:ss");
    }
    else
    {
        str += tr("\nmoving: ") + time.toString("HH:mm:ss");
    }

    IUnit::self().meter2speed(distance / ttime, val1, unit1);
    str += tr(", speed: %1 %2").arg(val1).arg(unit1);

    // ---------------------------
    str += tr("\nstart: %1").arg(getStartTimestamp().isNull() ? tr("-") : getStartTimestamp().toString());
    str += tr("\nend: %1").arg(getEndTimestamp().isNull() ? tr("-") : getEndTimestamp().toString());

    IUnit::self().meter2elevation(getAscend(), val1, unit1);
    IUnit::self().meter2elevation(getDescend(), val2, unit2);

    str += tr("\n%1%2 %3, %4%5 %6").arg(QChar(0x2197)).arg(val1).arg(unit1).arg(QChar(0x2198)).arg(val2).arg(unit2);

    return str;
}


/// set the icon defined by a string
void CTrack::setIcon(const QString& str)
{
    iconPixmap = QPixmap(16,16);
    iconPixmap.fill(str);

    iconString = str;
}


QString CTrack::getTrkPtInfo1(pt_t& trkpt)
{
    QString str, val, unit;

    // timestamp
    if(trkpt.timestamp != 0x00000000 && trkpt.timestamp != 0xFFFFFFFF)
    {
        QDateTime time = QDateTime::fromTime_t(trkpt.timestamp);
        if(!timezone.isEmpty())
        {
            time = TimeStamp(trkpt.timestamp).toZone(timezone).toDateTime();
        }

        str = time.toString();

    }

    // time to start and time to end
    if(trkpt.timestamp != 0x00000000 && trkpt.timestamp != 0xFFFFFFFF)
    {
        quint32 total = getTotalTime();
        if(total)
        {
            quint32 t1s = trkpt.timeSinceStart;
            quint32 t2s = total - trkpt.timeSinceStart;

            quint32 t1hh = t1s/3600;
            quint32 t2hh = t2s/3600;

            quint32 t1mm = (t1s - t1hh * 3600)/60;
            quint32 t2mm = (t2s - t2hh * 3600)/60;

            quint32 t1ss = t1s%60;
            quint32 t2ss = t2s%60;

            quint32 t1p = quint32(qreal(100 * t1s) / total + 0.5);
            quint32 t2p = 100 - t1p;

            str += "\n";
#ifndef WIN32
            str += tr("%5 %4 %1:%2:%3 (%6%)").arg(t1hh, 2, 10, QChar('0')).arg(t1mm, 2, 10, QChar('0')).arg(t1ss, 2, 10, QChar('0')).arg(QChar(0x21A4)).arg(QChar(0x2690)).arg(t1p);
            str += tr(" .. (%6%) %1:%2:%3 %4 %5").arg(t2hh, 2, 10, QChar('0')).arg(t2mm, 2, 10, QChar('0')).arg(t2ss, 2, 10, QChar('0')).arg(QChar(0x21A6)).arg(QChar(0x2691)).arg(t2p);
#else
            //Unicode character 0x2690 "WHITE FLAG" is not supported for Windows
            str += tr("%4 %1:%2:%3 (%5%)").arg(t1hh, 2, 10, QChar('0')).arg(t1mm, 2, 10, QChar('0')).arg(t1ss, 2, 10, QChar('0')).arg(QChar(0x21A4)).arg(t1p);
            //Unicode character 0x2691 "BLACK FLAG" is not supported for Windows
            str += tr(" .. (%5%) %1:%2:%3 %4").arg(t2hh, 2, 10, QChar('0')).arg(t2mm, 2, 10, QChar('0')).arg(t2ss, 2, 10, QChar('0')).arg(QChar(0x21A6)).arg(t2p);
#endif
        }

    }

    // distance to start and distance to end
    if(str.count()) str += "\n";
    IUnit::self().meter2distance(trkpt.distance, val, unit);
#ifndef WIN32
    str += tr("%5 %4 %1%2 (%3%)").arg(val).arg(unit).arg(trkpt.distance * 100.0 / getTotalDistance(),0,'f',0).arg(QChar(0x21A4)).arg(QChar(0x2690));
#else
    //Unicode character 0x2690 "WHITE FLAG" is not supported for Windows
    str += tr("%4 %1%2 (%3%)").arg(val).arg(unit).arg(trkpt.distance * 100.0 / getTotalDistance(),0,'f',0).arg(QChar(0x21A4));
#endif
    IUnit::self().meter2distance(getTotalDistance() - trkpt.distance, val, unit);
#ifndef WIN32
    str += tr(" .. (%3%) %1%2 %4 %5").arg(val).arg(unit).arg((getTotalDistance() - trkpt.distance) * 100.0 / getTotalDistance(),0,'f',0).arg(QChar(0x21A6)).arg(QChar(0x2691));
#else
    //Unicode character 0x2691 "BLACK FLAG" is not supported for Windows
    str += tr(" .. (%3%) %1%2 %4").arg(val).arg(unit).arg((getTotalDistance() - trkpt.distance) * 100.0 / getTotalDistance(),0,'f',0).arg(QChar(0x21A6));
#endif

    // elevation of point
    if(trkpt.ele != WPT_NOFLOAT)
    {
        if(str.count()) str += "\n";
        IUnit::self().meter2elevation(trkpt.ele, val, unit);
        str += tr("elevation: %1%2").arg(val).arg(unit);
    }

    if(trkpt.slope2 != WPT_NOFLOAT)
    {
        if(str.count()) str += " ";
        str += tr("slope: %1\260").arg(trkpt.slope2,0,'f',0);
    }

    if((trkpt.heartReateBpm != -1) || (trkpt.cadenceRpm != -1))
    {
        if(str.count()) str += "\n";
    }

    if(trkpt.heartReateBpm != -1)
    {
        str += tr("heart rate: %1bpm").arg(trkpt.heartReateBpm);
    }

    if((trkpt.heartReateBpm != -1) && (trkpt.cadenceRpm != -1))
    {
        str += " | ";
    }

    if(trkpt.cadenceRpm != -1)
    {
        str += tr("cadence: %1rpm").arg(trkpt.cadenceRpm);
    }

    //-----------------------------------------------------------------------------------------------------------
    //TODO: HOVERTEXT FOR EXTENSIONS
//#ifdef GPX_EXTENSIONS
    if (!trkpt.gpx_exts.values.isEmpty())
    {
        QList<QString> ext_list = trkpt.gpx_exts.values.keys();
        QString ex_name, ex_val;

        for(int i=0; i < trkpt.gpx_exts.values.size(); ++i)
        {
            ex_name = ext_list.value(i);
            ex_val = trkpt.gpx_exts.getValue(ex_name);

            if (ex_val != "") {str += tr("\n %1: %2 ").arg(ex_name).arg(ex_val);}

        }

    }
//#endif

    return str;
}


QString CTrack::getTrkPtInfo2(pt_t& trkpt)
{
    QString str, val, unit;

    if(CResources::self().showTrackProfileEleInfo())
    {
        // distance ascend descend in current stage
                                 /* = QChar(0x2690);*/
        QString lastName = tr("Start");
        double lastDist  = 0;
        double lastAsc   = 0;
        double lastDesc  = 0;

                                 /* = QChar(0x2691);*/
        QString nextName = tr("End");
        double nextDist  = getTotalDistance();
        double nextAsc   = getAscend();
        double nextDesc  = getDescend();

        foreach(const wpt_t& wpt, waypoints)
        {
            if(trkpt.distance < wpt.trkpt.distance)
            {
                nextDist = wpt.trkpt.distance;
                nextAsc  = wpt.trkpt.ascend;
                nextDesc = wpt.trkpt.descend;
                nextName = wpt.wpt->getName();
                break;
            }

            lastDist = wpt.trkpt.distance;
            lastAsc  = wpt.trkpt.ascend;
            lastDesc = wpt.trkpt.descend;
            lastName = wpt.wpt->getName();
        }

        if(!waypoints.isEmpty())
        {
            double delta, ascend, descend;

            if(!lastName.isEmpty())
            {
                delta    = trkpt.distance - lastDist;
                ascend   = trkpt.ascend   - lastAsc;
                descend  = trkpt.descend  - lastDesc;

                str += lastName + ":";
                IUnit::self().meter2distance(delta, val, unit);
                str += tr(" %3 %1 %2").arg(val).arg(unit).arg(QChar(0x21A4));
                IUnit::self().meter2elevation(ascend, val, unit);
                str += tr(" %3 %1 %2").arg(val).arg(unit).arg(QChar(0x2197));
                IUnit::self().meter2elevation(descend, val, unit);
                str += tr(" %3 %1 %2 ").arg(val).arg(unit).arg(QChar(0x2198));
            }

            if(!nextName.isEmpty())
            {
                delta    = nextDist - trkpt.distance;
                ascend   = nextAsc - trkpt.ascend;
                descend  = nextDesc - trkpt.descend;

                IUnit::self().meter2elevation(ascend, val, unit);
                str += tr("| %3 %1 %2").arg(val).arg(unit).arg(QChar(0x2197));
                IUnit::self().meter2elevation(descend, val, unit);
                str += tr(" %3 %1 %2").arg(val).arg(unit).arg(QChar(0x2198));
                IUnit::self().meter2distance(delta, val, unit);
                str += tr(" %1 %2").arg(val).arg(unit);
                str += tr(" %1 :%2").arg(QChar(0x21A6)).arg(nextName);
            }
        }
    }

    return str;
}

QString CTrack::getFocusInfo()
{
    double tmp, d;
    QString str, val, unit;
    QList<pt_t> focus;
    getPointOfFocus(focus);

    if(focus.size() < 2)
    {
        return str;
    }

    const pt_t& p1 = focus.first();
    const pt_t& p2 = focus.last();

    d = tmp = p2.distance - p1.distance;
    IUnit::self().meter2distance(tmp, val, unit);
    str += QString("%3 %1%2\n").arg(val).arg(unit).arg(QChar(0x21A6));
    if(p1.timestamp != 0x00000000 && p1.timestamp != 0xFFFFFFFF)
    {
        quint32 t  = p2.timestamp - p1.timestamp;
        quint32 hh = t / 3600;
        quint32 mm = (t - hh * 3600) / 60;
        quint32 ss = (t - hh * 3600 - mm * 60);

        str += QString("%4 %1:%2:%3\n").arg(hh,2,10,QChar('0')).arg(mm,2,10,QChar('0')).arg(ss,2,10,QChar('0')).arg(QChar(0x231a));
    }
    tmp = p2.ascend - p1.ascend;
    IUnit::self().meter2elevation(tmp, val, unit);
    str += QString("%3 %1%2 (%4\260)\n").arg(val).arg(unit).arg(QChar(0x2197)).arg(qRound(atan(tmp/d) * 360 / (2*M_PI)));
    tmp = p1.descend - p2.descend;
    IUnit::self().meter2elevation(tmp, val, unit);
    str += QString("%3 %1%2 (%4\260)").arg(val).arg(unit).arg(QChar(0x2198)).arg(qRound(atan(tmp/d) * 360 / (2*M_PI)));



    return str;
}

void CTrack::setDoScaleWpt2Track(Qt::CheckState state)
{
    doScaleWpt2Track = state;
    emit sigChanged();
}


void CTrack::slotScaleWpt2Track()
{
    CWptDB& wptdb = CWptDB::self();

    waypoints.clear();
    if(wptdb.count() == 0 )
    {
        return ;
    }

    IMap& map = CMapDB::self().getMap();
    if(doScaleWpt2Track == Qt::Unchecked)
    {
        CTrackDB::self().emitSigChanged();
        return ;
    }

    if(doScaleWpt2Track == Qt::PartiallyChecked )
    {
        if(wptdb.count() > 1000 || track.count() > 10000)
        {
            QString msg = tr(
                "You are trying to find waypoints along a track with %1 waypoints and a track of size %2. "
                "This can be a very time consuming operation. Go on?\n\n"
                "Your selection will be stored in the track's data. You can save it along with the data. "
                "To change the selection use the checkbox in the track edit dialog."
                ).arg(wptdb.count()).arg(track.count());

            QMessageBox::Button res = QMessageBox::warning(0,tr("Warning..."), msg, QMessageBox::Ok| QMessageBox::Abort, QMessageBox::Ok);

            if(res == QMessageBox::Abort)
            {
                doScaleWpt2Track = Qt::Unchecked;
                emit sigChanged();
                return ;
            }
            else
            {
                doScaleWpt2Track = Qt::Checked;
                emit sigChanged();
            }
        }
    }
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QMap<QString,CWpt*>::const_iterator w = wptdb.begin();
    while(w != wptdb.end())
    {
        wpt_t wpt;
        wpt.wpt    = (*w);
        wpt.x      = wpt.wpt->lon * DEG_TO_RAD;
        wpt.y      = wpt.wpt->lat * DEG_TO_RAD;

        map.convertRad2M(wpt.x, wpt.y);

        waypoints << wpt;

        ++w;
    }

    QList<pt_t>& trkpts                 = getTrackPoints();
    QList<pt_t>::const_iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        if(trkpt->flags & pt_t::eDeleted)
        {
            ++trkpt;
            continue;
        }

        double x = trkpt->lon * DEG_TO_RAD;
        double y = trkpt->lat * DEG_TO_RAD;
        map.convertRad2M(x, y);

        QList<wpt_t>::iterator wpt = waypoints.begin();
        while(wpt != waypoints.end())
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

    double minDist = WPT_TO_TRACK_DIST;
    if(map.isLonLat())
    {
        minDist = 6.50375e-10;
    }

    QList<CTrack::wpt_t>::iterator wpt = waypoints.begin();
    while(wpt != waypoints.end())
    {

        if(wpt->d > minDist)
        {
            wpt = waypoints.erase(wpt);
            continue;
        }
        ++wpt;
    }

    qSort(waypoints.begin(), waypoints.end(), qSortWptLessDistance);


    QApplication::restoreOverrideCursor();
    return ;
}


void CTrack::medianFilter(qint32 len, QProgressDialog& progress)
{
    cntMedianFilterApplied = (len - 5) >> 1;

    QList<float> window;
    for(qint32 i = 0; i<len; i++)
    {
        window << 0.0;
    }

    QList<CTrack::pt_t>::iterator trkpt, end;
    setupIterators(trkpt, end);

    QList<float> ele;

    if(cntMedianFilterApplied)
    {
        while(trkpt != end)
        {
            ele << trkpt->ele;
            trkpt++;
        }
    }
    else
    {
        while(trkpt != end)
        {
            ele << trkpt->_ele;
            trkpt++;
        }
    }

    setupIterators(trkpt, end);
    for(int i = (len>>1); i < (ele.size()-(len>>1)); i++)
    {
        // apply median filter over all trackpoints
        for(qint32 n = 0; n < len; n++)
        {
            window[n] = ele[i - (len>>1) + n];
        }

        qSort(window);
        trkpt[i].ele = window[(len>>1)];

        progress.setValue(i);
        qApp->processEvents();
        if (progress.wasCanceled())
        {
            return;
        }
    }

}

void CTrack::offsetElevation(double offset)
{

    QList<CTrack::pt_t>::iterator trkpt, end;
    setupIterators(trkpt, end);

    while(trkpt != end)
    {
        if(trkpt->ele != WPT_NOFLOAT)
        {
            trkpt->ele += offset;
        }

        trkpt++;
    }
}

void CTrack::changeStartTime(QDateTime& time)
{

    qint32 offset = time.toUTC().toTime_t() - getStartTimestamp().toTime_t();

    QList<CTrack::pt_t>::iterator trkpt, end;
    setupIterators(trkpt, end);

    while(trkpt != end)
    {
        if(trkpt->ele != WPT_NOFLOAT)
        {
            trkpt->timestamp += offset;
        }

        trkpt++;
    }
}

void CTrack::changeSpeed(double speed)
{
    double deltaD;
    double t1;
    double dT = 0, dTsec, dTmsec;
    double d1;
    QList<CTrack::pt_t>::iterator trkpt, end,trkpt1, trkpt2;

    trkpt   = getTrackPoints().begin();
    end     = getTrackPoints().end();
    setupIterators(trkpt1, trkpt2);

    t1 = trkpt->timestamp + double(trkpt->timestamp_msec)/1000;
    d1 = trkpt->distance;
    while(trkpt != trkpt1)
    {
        if(!(trkpt->flags & pt_t::eDeleted))
        {
            t1 = trkpt->timestamp + double(trkpt->timestamp_msec)/1000;
            d1 = trkpt->distance;
        }
        trkpt++;
    }

    while(trkpt != trkpt2)
    {

        if(trkpt->flags & pt_t::eDeleted)
        {
            trkpt++;
            continue;
        }

        deltaD   = trkpt->distance - d1;

        t1 = t1 + deltaD/speed;
        dT = t1 - trkpt->timestamp - double(trkpt->timestamp_msec)/1000;

        trkpt->timestamp        = floor(t1);
        trkpt->timestamp_msec   = (t1 - floor(t1)) * 1000;

        d1 = trkpt->distance;
        trkpt++;
    }

    dTsec  = floor(dT);
    dTmsec = dT - floor(dT);
    while(trkpt != end)
    {
        trkpt->timestamp        += dTsec;
        trkpt->timestamp_msec   += dTmsec;
        trkpt++;
    }
}

bool CTrack::unifyTimestamps(quint32 delta)
{
    QList<CTrack::pt_t>::iterator trkpt, end;
    trkpt   = getTrackPoints().begin();
    end     = getTrackPoints().end();

    if(delta > 0)
    {
        quint32 timestamp = trkpt->timestamp;

        if(timestamp == 0 || timestamp == 0xFFFFFFFF)
        {
            QMessageBox::warning(0,tr("Error..."),tr("This track has no valid start timestamp. Use the 'Date/Time' track filter to set one."), QMessageBox::Abort, QMessageBox::Abort);
            return false;
        }

        while(trkpt != end)
        {
            trkpt->timestamp        = timestamp;
            trkpt->timestamp_msec   = 0;

            timestamp += delta;
            trkpt++;
        }

    }
    else
    {
        while(trkpt != end)
        {
            trkpt->timestamp        = 0xFFFFFFFF;
            trkpt->timestamp_msec   = 0;
            trkpt++;
        }
    }

    return true;
}

void CTrack::reset()
{
    QList<CTrack::pt_t>::iterator trkpt, end;
    setupIterators(trkpt, end);

    while(trkpt != end)
    {
        trkpt->flags &= ~CTrack::pt_t::eDeleted;
        trkpt->lon = trkpt->_lon;
        trkpt->lat = trkpt->_lat;
        trkpt->ele = trkpt->_ele;
        trkpt->timestamp = trkpt->_timestamp;
        trkpt->timestamp_msec = trkpt->_timestamp_msec;

        ++trkpt;
    }

    cntMedianFilterApplied = 0;
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
