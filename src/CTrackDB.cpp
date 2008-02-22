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

#include "CTrackDB.h"
#include "CTrack.h"
#include "CTrackToolWidget.h"
#include "CQlb.h"
#include "CGpx.h"
#include "CResources.h"


#include <QtGui>

CTrackDB * CTrackDB::m_self = 0;

CTrackDB::CTrackDB(QToolBox * tb, QObject * parent)
    : IDB(tb,parent)
    , cnt(0)
{
    m_self      = this;
    toolview    = new CTrackToolWidget(tb);
}

CTrackDB::~CTrackDB()
{

}

void CTrackDB::loadGPX(CGpx& gpx)
{
    const QDomNodeList& trks = gpx.elementsByTagName("trk");
    uint N = trks.count();
    for(uint n = 0; n < N; ++n) {
        const QDomNode& trk = trks.item(n);

        CTrack * track = new CTrack(this);

        if(trk.namedItem("name").isElement()) {
            track->setName(trk.namedItem("name").toElement().text());
        }

        if(trk.namedItem("extension").isElement()) {
            const QDomNode& ext = trk.namedItem("extension");
            if(ext.namedItem("color").isElement()) {
                track->setColor(ext.namedItem("color").toElement().text().toUInt());
            }
        }

        const QDomNode& trkseg = trk.namedItem("trkseg");
        QDomElement trkpt = trkseg.firstChildElement("trkpt");

        while (!trkpt.isNull()) {
            CTrack::pt_t pt;

            QDomNamedNodeMap attr = trkpt.attributes();

            pt.lon = attr.namedItem("lon").nodeValue().toDouble();
            pt.lat = attr.namedItem("lat").nodeValue().toDouble();

            if(trkpt.namedItem("ele").isElement()) {
                pt.ele = trkpt.namedItem("ele").toElement().text().toDouble();
            }

            if(trkpt.namedItem("time").isElement()) {
                QDateTime time = QDateTime::fromString(trkpt.namedItem("time").toElement().text(),"yyyy-MM-dd'T'hh:mm:ss'Z'");
                time.setTimeSpec(Qt::UTC);
                pt.time = time.toTime_t() - CResources::self().getUTCOffset();
            }

            if(trkpt.namedItem("extension").isElement()) {
                const QDomNode& ext = trkpt.namedItem("extension");
                if(ext.namedItem("flags").isElement()) {
                    pt.flags = ext.namedItem("flags").toElement().text().toUInt();
                    pt.flags &= ~CTrack::pt_t::eFocus;
                    pt.flags &= ~CTrack::pt_t::eSelected;
                    pt.flags &= ~CTrack::pt_t::eCursor;
                }
            }

            *track << pt;
            trkpt = trkpt.nextSiblingElement("trkpt");
        }

        if(track->getName().isEmpty()){
            track->setName(tr("Track%1").arg(cnt++));
        }
        track->rebuild(false);
        delTrack(track->key());
        tracks[track->key()] = track;

        connect(track,SIGNAL(sigChanged()),SIGNAL(sigChanged()));

    }
    emit sigChanged();

}

void CTrackDB::delTrack(const QString& key, bool silent)
{
    if(!tracks.contains(key)) return;
    delete tracks.take(key);
    if(!silent) emit sigChanged();
}

void CTrackDB::delTracks(const QStringList& keys)
{
    QString key;
    foreach(key,keys) {
        if(!tracks.contains(key)) continue;
        delete tracks.take(key);
    }
    emit sigChanged();
}


void CTrackDB::highlightTrack(const QString& key)
{
    QMap<QString,CTrack*>::iterator track = tracks.begin();
    while(track != tracks.end()) {
        (*track)->setHighlight(false);
        ++track;
    }

    tracks[key]->setHighlight(true);
    emit sigChanged();

}
