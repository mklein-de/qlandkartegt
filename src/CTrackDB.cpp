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
#include "IDevice.h"

#include <QtGui>

CTrackDB * CTrackDB::m_self = 0;

CTrackDB::CTrackDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
, cnt(0)
{
    m_self      = this;
    toolview    = new CTrackToolWidget(tb);
}


CTrackDB::~CTrackDB()
{

}


void CTrackDB::clear()
{
    cnt = 0;
    delTracks(tracks.keys());
    emit sigChanged();
}


CTrackToolWidget * CTrackDB::getToolWidget()
{
    return qobject_cast<CTrackToolWidget*>(toolview);
}


void CTrackDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.tracks(),QIODevice::ReadOnly);

    while(!stream.atEnd()) {
        CTrack * track = new CTrack(this);
        stream >> *track;
        addTrack(track);
    }

    emit sigChanged();

}


void CTrackDB::saveQLB(CQlb& qlb)
{
    QMap<QString, CTrack*>::const_iterator track = tracks.begin();
    while(track != tracks.end()) {
        qlb << *(*track);
        ++track;
    }
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
                pt.timestamp = time.toTime_t();
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

        if(track->getTrackPoints().count() > 0){
            addTrack(track);
        }
    }
    emit sigChanged();
}


void CTrackDB::saveGPX(CGpx& gpx)
{
    QDomElement root = gpx.documentElement();
    QMap<QString,CTrack*>::iterator track = tracks.begin();
    while(track != tracks.end()) {
        QDomElement trk = gpx.createElement("trk");
        root.appendChild(trk);

        QDomElement name = gpx.createElement("name");
        trk.appendChild(name);
        QDomText _name_ = gpx.createTextNode((*track)->getName());
        name.appendChild(_name_);

        QDomElement ext = gpx.createElement("extension");
        trk.appendChild(ext);

        QDomElement color = gpx.createElement("color");
        ext.appendChild(color);
        QDomText _color_ = gpx.createTextNode(QString::number((*track)->getColorIdx()));
        color.appendChild(_color_);

        QDomElement trkseg = gpx.createElement("trkseg");
        trk.appendChild(trkseg);

        QVector<CTrack::pt_t>& pts = (*track)->getTrackPoints();
        QVector<CTrack::pt_t>::const_iterator pt = pts.begin();
        while(pt != pts.end()) {
            QDomElement trkpt = gpx.createElement("trkpt");
            trkseg.appendChild(trkpt);
            trkpt.setAttribute("lat",(double)pt->lat);
            trkpt.setAttribute("lon",(double)pt->lon);

            if(pt->ele != WPT_NOFLOAT) {
                QDomElement ele = gpx.createElement("ele");
                trkpt.appendChild(ele);
                QDomText _ele_ = gpx.createTextNode(QString::number(pt->ele));
                ele.appendChild(_ele_);
            }
            if(pt->timestamp != 0x000000000 && pt->timestamp != 0xFFFFFFFF) {
                QDateTime t = QDateTime::fromTime_t(pt->timestamp).toUTC();
                QDomElement time = gpx.createElement("time");
                trkpt.appendChild(time);
                QDomText _time_ = gpx.createTextNode(t.toString("yyyy-MM-dd'T'hh:mm:ss'Z'"));
                time.appendChild(_time_);
            }

            QDomElement extension = gpx.createElement("extension");
            trkpt.appendChild(extension);

            QDomElement flags = gpx.createElement("flags");
            extension.appendChild(flags);
            QDomText _flags_ = gpx.createTextNode(QString::number(pt->flags));
            flags.appendChild(_flags_);

            ++pt;
        }

        ++track;
    }

}


void CTrackDB::addTrack(CTrack* track)
{
    if(track->getName().isEmpty()) {
        track->setName(tr("Track%1").arg(cnt++));
    }
    track->rebuild(false);
    delTrack(track->key());
    tracks[track->key()] = track;

    connect(track,SIGNAL(sigChanged()),SIGNAL(sigChanged()));

}


void CTrackDB::delTrack(const QString& key, bool silent)
{
    if(!tracks.contains(key)) return;
    delete tracks.take(key);
    if(!silent) emit sigChanged();
    emit sigModified();
}


void CTrackDB::delTracks(const QStringList& keys)
{
    QString key;
    foreach(key,keys) {
        if(!tracks.contains(key)) continue;
        delete tracks.take(key);
    }
    emit sigChanged();
    emit sigModified();
}


void CTrackDB::highlightTrack(const QString& key)
{
    QMap<QString,CTrack*>::iterator track = tracks.begin();
    while(track != tracks.end()) {
        (*track)->setHighlight(false);
        ++track;
    }

    tracks[key]->setHighlight(true);

    emit sigHighlightTrack(tracks[key]);
    emit sigChanged();

}


CTrack* CTrackDB::highlightedTrack()
{

    QMap<QString,CTrack*>::iterator track = tracks.begin();
    while(track != tracks.end()) {
        if((*track)->isHighlighted()) return *track;
        ++track;
    }
    return 0;

}

void CTrackDB::download()
{
    IDevice * dev = CResources::self().device();
    if(dev) {
        QList<CTrack*> tmptrk;
        dev->downloadTracks(tmptrk);

        if(tmptrk.isEmpty()) return;

        CTrack * trk;
        foreach(trk,tmptrk) {
            addTrack(trk);
        }
    }

    emit sigChanged();
    emit sigModified();
}
