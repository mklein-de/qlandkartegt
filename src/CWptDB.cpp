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
#include "CResources.h"
#include "IDevice.h"


#include <QtGui>

CWptDB * CWptDB::m_self;

CWptDB::CWptDB(QToolBox * tb, QObject * parent)
    : IDB(tb,parent)
{
    m_self      = this;
    toolview    = new CWptToolWidget(tb);

//     CWpt * wpt = new CWpt(this);
//     QFile file("2008.01.21_09.44.36_xxx.wpt");
//     file.open(QIODevice::ReadOnly);
//     QDataStream in(&file);
//     in >> *wpt;
//     file.close();
//     wpts[wpt->key()] = wpt;
//
//     emit sigChanged();

}

CWptDB::~CWptDB()
{

}

void CWptDB::newWpt(double lon, double lat)
{
    CWpt * wpt = new CWpt(this);
    wpt->lon = lon;
    wpt->lat = lat;

    QSettings cfg;
    wpt->icon = cfg.value("waypoint/lastSymbol","Star").toString();

    CDlgEditWpt dlg(*wpt,theMainWindow->getCanvas());
    if(dlg.exec() == QDialog::Rejected){
        delete wpt;
        return;
    }
    wpts[wpt->key()] = wpt;

//     QFile file(wpt->filename());
//     file.open(QIODevice::WriteOnly);
//     QDataStream out(&file);
//     out << *wpt;
//     file.close();

    emit sigChanged();
}

CWpt * CWptDB::getWptByKey(const QString& key)
{
    if(!wpts.contains(key)) return 0;

    return wpts[key];

}

void CWptDB::loadGPX(CGpx& gpx)
{
}

void CWptDB::saveGPX(CGpx& gpx)
{
}

void CWptDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.waypoints(),QIODevice::ReadOnly);

    while(!stream.atEnd()){
        CWpt * wpt = new CWpt(this);

        stream >> *wpt;
        qDebug() << wpt->name;
        wpts[wpt->key()] = wpt;
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

}

