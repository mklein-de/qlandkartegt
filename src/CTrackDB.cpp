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
#include "CDlgCombineTracks.h"
#include "CMapDB.h"
#include "IMap.h"

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


QRectF CTrackDB::getBoundingRectF(const QString key)
{
    if(!tracks.contains(key)) {
        return QRectF();
    }
    return tracks.value(key)->getBoundingRectF();
}


QRectF CTrackDB::getBoundingRectF()
{
    QRectF r;
    foreach(CTrack *track, tracks.values()) {
        r = r.united(track->getBoundingRectF());
    }
    return r;
}


void CTrackDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.tracks(),QIODevice::ReadOnly);

    while(!stream.atEnd()) {
        CTrack * track = new CTrack(this);
        stream >> *track;
        addTrack(track, true);
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
                QDateTime time;
                if ( trkpt.namedItem("time").toElement().text().indexOf(".") != -1 )
                    time = QDateTime::fromString(trkpt.namedItem("time").toElement().text(),"yyyy-MM-dd'T'hh:mm:ss.zzz'Z'");
                else
                    time = QDateTime::fromString(trkpt.namedItem("time").toElement().text(),"yyyy-MM-dd'T'hh:mm:ss'Z'");
                time.setTimeSpec(Qt::UTC);
                pt.timestamp = time.toTime_t();
                pt.timestamp_msec = time.time().msec();
            }

            if(trkpt.namedItem("hdop").isElement()) {
                pt.hdop = trkpt.namedItem("hdop").toElement().text().toDouble();
            }

            if(trkpt.namedItem("vdop").isElement()) {
                pt.vdop = trkpt.namedItem("vdop").toElement().text().toDouble();
            }

            if(trkpt.namedItem("pdop").isElement()) {
                pt.pdop = trkpt.namedItem("pdop").toElement().text().toDouble();
            }

            // import from GPX 1.0
            if(trkpt.namedItem("course").isElement()) {
                pt.heading = trkpt.namedItem("course").toElement().text().toDouble();
            }
            // import from GPX 1.0
            if(trkpt.namedItem("speed").isElement()) {
                pt.velocity = trkpt.namedItem("speed").toElement().text().toDouble();
            }

            if(trkpt.namedItem("fix").isElement()) {
                pt.fix = trkpt.namedItem("fix").toElement().text();
            }

            if(trkpt.namedItem("sat").isElement()) {
                pt.sat = trkpt.namedItem("sat").toElement().text().toUInt();
            }

            if(trkpt.namedItem("extension").isElement()) {
                const QDomNode& ext = trkpt.namedItem("extension");
                if(ext.namedItem("flags").isElement()) {
                    pt.flags = ext.namedItem("flags").toElement().text().toUInt();
                    pt.flags &= ~CTrack::pt_t::eFocus;
                    pt.flags &= ~CTrack::pt_t::eSelected;
                    pt.flags &= ~CTrack::pt_t::eCursor;
                }
                if(trkpt.namedItem("rmc:course").isElement()) {
                    pt.heading = trkpt.namedItem("rmc:course").toElement().text().toDouble();
                }
                if(trkpt.namedItem("rmc:speed").isElement()) {
                    pt.velocity = trkpt.namedItem("rmc:speed").toElement().text().toDouble();
                }
            }

            *track << pt;
            trkpt = trkpt.nextSiblingElement("trkpt");
        }

        if(track->getTrackPoints().count() > 0) {
            addTrack(track, true);
        }
        else {
            delete track;
        }
    }
    emit sigChanged();
}


void CTrackDB::saveGPX(CGpx& gpx)
{
    QString str;
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

        QList<CTrack::pt_t>& pts = (*track)->getTrackPoints();
        QList<CTrack::pt_t>::const_iterator pt = pts.begin();
        while(pt != pts.end()) {
            QDomElement trkpt = gpx.createElement("trkpt");
            trkseg.appendChild(trkpt);
            str.sprintf("%1.8f", pt->lat);
            trkpt.setAttribute("lat",str);
            str.sprintf("%1.8f", pt->lon);
            trkpt.setAttribute("lon",str);

            if(pt->ele != WPT_NOFLOAT) {
                QDomElement ele = gpx.createElement("ele");
                trkpt.appendChild(ele);
                QDomText _ele_ = gpx.createTextNode(QString::number(pt->ele));
                ele.appendChild(_ele_);
            }

            if(pt->timestamp != 0x000000000 && pt->timestamp != 0xFFFFFFFF) {
                QDateTime t = QDateTime::fromTime_t(pt->timestamp).toUTC();
                t = t.addMSecs(pt->timestamp_msec);
                QDomElement time = gpx.createElement("time");
                trkpt.appendChild(time);
                QDomText _time_ = gpx.createTextNode(t.toString("yyyy-MM-dd'T'hh:mm:ss.zzz'Z'"));
                time.appendChild(_time_);
            }

            if(pt->hdop != WPT_NOFLOAT) {
                QDomElement hdop = gpx.createElement("hdop");
                trkpt.appendChild(hdop);
                QDomText _hdop_ = gpx.createTextNode(QString::number(pt->hdop));
                hdop.appendChild(_hdop_);
            }
            if(pt->vdop != WPT_NOFLOAT) {
                QDomElement vdop = gpx.createElement("vdop");
                trkpt.appendChild(vdop);
                QDomText _vdop_ = gpx.createTextNode(QString::number(pt->vdop));
                vdop.appendChild(_vdop_);
            }
            if(pt->pdop != WPT_NOFLOAT) {
                QDomElement pdop = gpx.createElement("pdop");
                trkpt.appendChild(pdop);
                QDomText _pdop_ = gpx.createTextNode(QString::number(pt->pdop));
                pdop.appendChild(_pdop_);
            }

            if(pt->fix != "") {
                QDomElement fix = gpx.createElement("fix");
                trkpt.appendChild(fix);
                QDomText _fix_ = gpx.createTextNode(pt->fix);
                fix.appendChild(_fix_);
            }

            if(pt->sat != 0) {
                QDomElement sat = gpx.createElement("sat");
                trkpt.appendChild(sat);
                QDomText _sat_ = gpx.createTextNode(QString::number(pt->sat));
                sat.appendChild(_sat_);
            }

            // gpx extensions
            QDomElement extension = gpx.createElement("extension");
            trkpt.appendChild(extension);

            QDomElement flags = gpx.createElement("flags");
            extension.appendChild(flags);
            QDomText _flags_ = gpx.createTextNode(QString::number(pt->flags));
            flags.appendChild(_flags_);

            if(pt->heading != WPT_NOFLOAT) {
                QDomElement heading = gpx.createElement("rmc:course");
                extension.appendChild(heading);
                QDomText _heading_ = gpx.createTextNode(QString::number(pt->heading));
                heading.appendChild(_heading_);
            }

            if(pt->velocity != WPT_NOFLOAT) {
                QDomElement velocity = gpx.createElement("rmc:speed");
                extension.appendChild(velocity);
                QDomText _velocity_ = gpx.createTextNode(QString::number(pt->velocity));
                velocity.appendChild(_velocity_);
            }

            ++pt;
        }

        ++track;
    }

}


void CTrackDB::addTrack(CTrack* track, bool silent)
{
    if(track->getName().isEmpty()) {
        track->setName(tr("Track%1").arg(cnt++));
    }
    track->rebuild(false);
    delTrack(track->key(), silent);
    tracks[track->key()] = track;

    connect(track,SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    if(!silent) {
        emit sigChanged();
        emit sigModified();
    }
}


void CTrackDB::delTrack(const QString& key, bool silent)
{
    if(!tracks.contains(key)) return;
    delete tracks.take(key);
    if(!silent) {
        emit sigChanged();
        emit sigModified();
    }
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


void CTrackDB::upload()
{
    if(tracks.isEmpty()) return;

    IDevice * dev = CResources::self().device();
    if(dev) {
        QList<CTrack*> tmptrks = tracks.values();
        dev->uploadTracks(tmptrks);
    }
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
            addTrack(trk, true);
        }
    }

    emit sigChanged();
    emit sigModified();
}


void CTrackDB::CombineTracks()
{
    CDlgCombineTracks dlg(0);
    dlg.exec();
}


void CTrackDB::splitTrack(int idx)
{
    CTrack * theTrack = highlightedTrack();
    if(theTrack == 0) return;

    int i;
    QList<CTrack::pt_t>& track          = theTrack->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt = track.begin();

    CTrack * track1 = new CTrack(this);
    track1->setName(theTrack->getName() + "_1");
    for(i = 0; (i <= idx) && (trkpt != track.end()); ++i) {
        *track1 << *trkpt++;
    }

    CTrack * track2 = new CTrack(this);
    track2->setName(theTrack->getName() + "_2");
    for( ;(trkpt != track.end()); ++i) {
        *track2 << *trkpt++;
    }

    addTrack(track1, true);
    addTrack(track2, true);

    emit sigChanged();
    emit sigModified();
}


void CTrackDB::draw(QPainter& p, const QRect& rect, bool& needsRedraw)
{
    QPoint focus(-1,-1);
    QVector<QPoint> selected;
    IMap& map = CMapDB::self().getMap();
    //     QMap<QString,CTrack*> tracks                = CTrackDB::self().getTracks();
    QMap<QString,CTrack*>::iterator track       = tracks.begin();
    QMap<QString,CTrack*>::iterator highlighted = tracks.end();

    QPixmap bullet_red(":/icons/bullet_red.png");

    while(track != tracks.end()) {
        QPolygon& line = (*track)->getPolyline();
        line.clear();

        bool firstTime = (*track)->firstTime;

        QList<CTrack::pt_t>& trkpts = (*track)->getTrackPoints();
        QList<CTrack::pt_t>::iterator trkpt = trkpts.begin();
        while(trkpt != trkpts.end()) {

            if ( needsRedraw || firstTime) {
                double u = trkpt->lon * DEG_TO_RAD;
                double v = trkpt->lat * DEG_TO_RAD;

                map.convertRad2Pt(u,v);
                trkpt->px = QPoint(u,v);

            }

            if((*track)->isHighlighted() && trkpt->flags & CTrack::pt_t::eSelected) {
                selected << trkpt->px;
            }

            if((*track)->isHighlighted() && trkpt->flags & CTrack::pt_t::eFocus) {
                focus = trkpt->px;
            }

            // skip deleted points, however if they are selected the
            // selection mark is shown
            if(trkpt->flags & CTrack::pt_t::eDeleted) {
                ++trkpt; continue;
            }

            line << trkpt->px;
            ++trkpt;
        }

        if(!rect.intersects(line.boundingRect())) {
            ++track; continue;
        }

        if((*track)->isHighlighted()) {
            // store highlighted track to draw it later
            // it must be drawn above all other tracks
            highlighted = track;
        }
        else {
            // draw normal track
            QPen pen((*track)->getColor(),3);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            p.setPen(pen);
            p.drawPolyline(line);
            p.setPen(Qt::white);
            p.drawPolyline(line);
        }

        (*track)->firstTime = false;
        ++track;
    }

    // if there is a highlighted track, draw it
    if(highlighted != tracks.end()) {
        track = highlighted;

        QPolygon& line = (*track)->getPolyline();

        // draw skunk line
        QPen pen((*track)->getColor(),5);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        p.drawPolyline(line);
        p.setPen(Qt::white);
        p.drawPolyline(line);

        // draw bubbles
        QPoint pt, pt1;

        QPixmap bullet = (*track)->getBullet();
        foreach(pt,line) {
            if(abs((pt.x() - pt1.x()) + abs(pt.y() - pt1.y())) < 7) continue;
            p.drawPixmap(pt.x() - 3 ,pt.y() - 3, bullet);
            pt1 = pt;
        }

        pt1 = QPoint();
        foreach(pt,selected) {
            if(abs((pt.x() - pt1.x()) + abs(pt.y() - pt1.y())) < 7) continue;
            p.drawPixmap(pt.x() - 5 ,pt.y() - 5, bullet_red);
            pt1 = pt;
        }

        if(focus != QPoint(-1,-1)) {
            p.setPen(QPen(Qt::white,3));
            p.drawLine(focus + QPoint(0,-20),focus + QPoint(0,20));
            p.drawLine(focus + QPoint(-20,0),focus + QPoint(20,0));
            p.setPen(Qt::red);
            p.drawLine(focus + QPoint(0,-20),focus + QPoint(0,20));
            p.drawLine(focus + QPoint(-20,0),focus + QPoint(20,0));
        }
    }
}


void CTrackDB::select(const QRect& rect)
{
    CTrack * track = highlightedTrack();
    if(track == 0) return;

    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt = trkpts.begin();
    while(trkpt != trkpts.end()) {
        if(rect.contains(trkpt->px) && !(trkpt->flags & CTrack::pt_t::eDeleted)) {
            trkpt->flags |= CTrack::pt_t::eSelected;
        }

        ++trkpt;
    }

    emit track->sigChanged();
}
