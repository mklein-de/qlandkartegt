/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CWptDB.h"
#include "CWptToolWidget.h"
#include "CDlgEditWpt.h"
#include "CWpt.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CQlb.h"
#include "CGpx.h"
#include "CResources.h"
#include "IDevice.h"


#include <QtGui>


CWptDB * CWptDB::m_self = 0;

CWptDB::CWptDB(QTabWidget * tb, QObject * parent)
    : IDB(tb,parent)
{
    m_self      = this;
    toolview    = new CWptToolWidget(tb);

}

CWptDB::~CWptDB()
{

}

void CWptDB::newWpt(float lon, float lat, float ele)
{
    CWpt * wpt = new CWpt(this);
    wpt->lon = lon * RAD_TO_DEG;
    wpt->lat = lat * RAD_TO_DEG;
    wpt->ele = ele;

    QSettings cfg;
    wpt->icon = cfg.value("waypoint/lastSymbol","Star").toString();

    CDlgEditWpt dlg(*wpt,theMainWindow->getCanvas());
    if(dlg.exec() == QDialog::Rejected){
        delete wpt;
        return;
    }
    wpts[wpt->key()] = wpt;

    emit sigChanged();
}

CWpt * CWptDB::getWptByKey(const QString& key)
{
    if(!wpts.contains(key)) return 0;

    return wpts[key];

}


void CWptDB::delWpt(const QString& key, bool silent)
{
    if(!wpts.contains(key)) return;
    if(wpts[key]->sticky){
        QString msg = tr("Do you really want to delete the sticky waypoint '%1'").arg(wpts[key]->name);
        if(QMessageBox::question(0,tr("Delete sticky waypoint ..."),msg, QMessageBox::Ok|QMessageBox::No, QMessageBox::No) == QMessageBox::No){
            return;
        }
    }
    delete wpts.take(key);
    if(!silent) emit sigChanged();
}

void CWptDB::delWpt(const QStringList& keys)
{
    QString key;
    foreach(key, keys){
        delWpt(key,true);
    }

    emit sigChanged();
}


void CWptDB::addWpt(CWpt * wpt)
{
    if(wpts.contains(wpt->key())){
        delWpt(wpt->key(), true);
    }
    wpts[wpt->key()] = wpt;
}

void CWptDB::loadGPX(CGpx& gpx)
{
    const QDomNodeList& waypoints = gpx.elementsByTagName("wpt");
    uint N = waypoints.count();
    for(uint n = 0; n < N; ++n){
        const QDomNode& waypoint = waypoints.item(n);

        CWpt * wpt = new CWpt(this);

        const QDomNamedNodeMap& attr = waypoint.attributes();
        wpt->lon = attr.namedItem("lon").nodeValue().toDouble();
        wpt->lat = attr.namedItem("lat").nodeValue().toDouble();
        if(waypoint.namedItem("name").isElement()){
            wpt->name = waypoint.namedItem("name").toElement().text();
        }
        if(waypoint.namedItem("cmt").isElement()){
            wpt->comment = waypoint.namedItem("cmt").toElement().text();
        }
        if(waypoint.namedItem("desc").isElement()){
            wpt->comment = waypoint.namedItem("desc").toElement().text();
        }
        if(waypoint.namedItem("link").isElement()){
            const QDomNode& link = waypoint.namedItem("link");
            const QDomNamedNodeMap& attr = link.toElement().attributes();
            wpt->link = attr.namedItem("href").nodeValue();
        }
        if(waypoint.namedItem("url").isElement()){
            wpt->link = waypoint.namedItem("url").toElement().text();
        }
        if(waypoint.namedItem("sym").isElement()){
            wpt->icon =  waypoint.namedItem("sym").toElement().text();
        }
        if(waypoint.namedItem("ele").isElement()){
            wpt->ele = waypoint.namedItem("ele").toElement().text().toDouble();
        }
        if(waypoint.namedItem("time").isElement()){
            QDateTime time = QDateTime::fromString(waypoint.namedItem("time").toElement().text(),"yyyy-MM-dd'T'hh:mm:ss'Z'");
            time.setTimeSpec(Qt::UTC);
            wpt->timestamp = time.toTime_t();// - gpResources->getUTCOffset();
        }

        if(waypoint.namedItem("extension").isElement()){
            const QDomNode& ext = waypoint.namedItem("extension");
            if(ext.namedItem("dist").isElement()){
                wpt->prx = ext.namedItem("dist").toElement().text().toDouble();
            }
        }

        if(wpt->lat == 1000 || wpt->lon == 1000 || wpt->name.isEmpty()){
            delete wpt;
            continue;
        }

        addWpt(wpt);
    }

    emit sigChanged();
}

void CWptDB::saveGPX(CGpx& gpx)
{
    QDomElement root = gpx.documentElement();
    QMap<QString,CWpt*>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end()){
        if((*wpt)->sticky){
            ++wpt;
            continue;
        }
        QDomElement waypoint = gpx.createElement("wpt");
        root.appendChild(waypoint);
        waypoint.setAttribute("lat",(double)(*wpt)->lat);
        waypoint.setAttribute("lon",(double)(*wpt)->lon);

        if((*wpt)->ele != 1e25f){
            QDomElement ele = gpx.createElement("ele");
            waypoint.appendChild(ele);
            QDomText _ele_ = gpx.createTextNode(QString::number((*wpt)->ele));
            ele.appendChild(_ele_);
        }


        QDateTime t = QDateTime::fromTime_t((*wpt)->timestamp);
        QDomElement time = gpx.createElement("time");
        waypoint.appendChild(time);
        QDomText _time_ = gpx.createTextNode(t.toString("yyyy-MM-dd'T'hh:mm:ss'Z'"));
        time.appendChild(_time_);

        QDomElement name = gpx.createElement("name");
        waypoint.appendChild(name);
        QDomText _name_ = gpx.createTextNode((*wpt)->name);
        name.appendChild(_name_);

        if(!(*wpt)->comment.isEmpty()){
            QDomElement cmt = gpx.createElement("cmt");
            waypoint.appendChild(cmt);
            QDomText _cmt_ = gpx.createTextNode((*wpt)->comment);
            cmt.appendChild(_cmt_);
        }

        if(!(*wpt)->link.isEmpty()){
            QDomElement link = gpx.createElement("link");
            waypoint.appendChild(link);
            link.setAttribute("href",(*wpt)->link);
            QDomElement text = gpx.createElement("text");
            link.appendChild(text);
            QDomText _text_ = gpx.createTextNode((*wpt)->name);
            text.appendChild(_text_);
        }

        QDomElement sym = gpx.createElement("sym");
        waypoint.appendChild(sym);
        QDomText _sym_ = gpx.createTextNode((*wpt)->icon);
        sym.appendChild(_sym_);

        if((*wpt)->prx != 1e25f){
            QDomElement extension = gpx.createElement("extension");
            waypoint.appendChild(extension);

            if((*wpt)->prx != 1e25f){
                QDomElement dist = gpx.createElement("dist");
                extension.appendChild(dist);
                QDomText _dist_ = gpx.createTextNode(QString::number((*wpt)->prx));
                dist.appendChild(_dist_);
            }
        }

        ++wpt;
    }
}

void CWptDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.waypoints(),QIODevice::ReadOnly);

    while(!stream.atEnd()){
        CWpt * wpt = new CWpt(this);
        stream >> *wpt;
        addWpt(wpt);
    }

    emit sigChanged();

}

void CWptDB::saveQLB(CQlb& qlb)
{
    QMap<QString, CWpt*>::const_iterator wpt = wpts.begin();
    while(wpt != wpts.end()){
        qlb << *(*wpt);
        ++wpt;
    }
}


void CWptDB::upload()
{
    if(wpts.isEmpty()) return;

    IDevice * dev = CResources::self().device();
    if(dev){
        QList<CWpt*> tmpwpts = wpts.values();
        dev->uploadWpts(tmpwpts);
    }

}

void CWptDB::download()
{
    IDevice * dev = CResources::self().device();
    if(dev){
        QList<CWpt*> tmpwpts;
        dev->downloadWpts(tmpwpts);

        CWpt * wpt;
        foreach(wpt,tmpwpts){
            addWpt(wpt);
        }
    }

    emit sigChanged();
}

void CWptDB::selWptByKey(const QString& key)
{
    CWptToolWidget * t = qobject_cast<CWptToolWidget*>(toolview);
    if(t){
        t->selWptByKey(key);
    }
}
