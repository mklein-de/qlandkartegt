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

#include "CMapDB.h"
#include "CMapToolWidget.h"
#include "CMapQMAP.h"
#include "CMapRaster.h"
#include "CMainWindow.h"
#include "CStatusCanvas.h"
#include "CMapEditWidget.h"

#include <QtGui>

CMapDB * CMapDB::m_self = 0;

CMapDB::CMapDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
{
    m_self      = this;
    toolview    = new CMapToolWidget(tb);

    defaultMap = new CMapNoMap(theMainWindow->getCanvas());

    QSettings cfg;
    QString map;
    QStringList maps = cfg.value("maps/knownMaps","").toString().split("|",QString::SkipEmptyParts);
    foreach(map, maps) {
        QSettings mapdef(map,QSettings::IniFormat);
        map_t m;
        m.filename    = map;
        m.description = mapdef.value("description/comment","").toString();
        if(m.description.isEmpty()) m.description = QFileInfo(map).fileName();
        m.key         = map;
        knownMaps[m.key] = m;
    }

    maps = cfg.value("maps/visibleMaps","").toString().split("|",QString::SkipEmptyParts);
    foreach(map, maps) {
        QFileInfo fi(map);
        QString ext = fi.suffix();
        if(ext == "qmap") {
            theMap = new CMapQMAP(map,theMainWindow->getCanvas());
        }
        else{
            theMap = new CMapRaster(map,theMainWindow->getCanvas());
        }
        //TODO: has to be removed for several layers
        break;
    }
    emit sigChanged();

    statusCanvas = new CStatusCanvas(theMainWindow->getCanvas());
    statusCanvas->updateShadingType();

    theMainWindow->statusBar()->insertPermanentWidget(0,statusCanvas);
}


CMapDB::~CMapDB()
{
    QSettings cfg;
    QString maps;
    map_t map;
    foreach(map,knownMaps) {
        maps += map.filename + "|";
    }
    cfg.setValue("maps/knownMaps",maps);
}


IMap& CMapDB::getMap() {
    return (theMap.isNull() ? *defaultMap : *theMap);
}

void CMapDB::closeVisibleMaps()
{
    if(!theMap.isNull()) delete theMap;
}


void CMapDB::openMap(const QString& filename, CCanvas& canvas)
{

    closeVisibleMaps();

    map_t map;
    QFileInfo fi(filename);
    QString ext = fi.suffix();
    if(ext == "qmap") {
        QSettings mapdef(filename,QSettings::IniFormat);
        map.filename    = filename;
        map.description = mapdef.value("description/comment","").toString();
        if(map.description.isEmpty()) map.description = fi.fileName();
        map.key         = filename;

        theMap = new CMapQMAP(filename,&canvas);

        knownMaps[map.key] = map;
        emit sigChanged();
    }
    else {
        theMap = new CMapRaster(filename,&canvas);
        emit sigChanged();
    }

    QSettings cfg;
//     QString maps;
//     foreach(imap,visibleMaps) {
//         maps += imap->getFilename();
//     }
    cfg.setValue("maps/visibleMaps",theMap->getFilename());

    statusCanvas->updateShadingType();



}


void CMapDB::openMap(const QString& key)
{
    if(!knownMaps.contains(key)) return;

    closeVisibleMaps();
    theMap = new CMapQMAP(knownMaps[key].filename,theMainWindow->getCanvas());

    QSettings cfg;
//     QString maps;
//     foreach(imap,visibleMaps) {
//         maps += imap->getFilename();
//     }
    cfg.setValue("maps/visibleMaps",theMap->getFilename());

    statusCanvas->updateShadingType();
}

void CMapDB::closeMap()
{
    closeVisibleMaps();
    QSettings cfg;
//     QString maps;
//     foreach(imap,visibleMaps) {
//         maps += imap->getFilename();
//     }
    cfg.setValue("maps/visibleMaps",theMap->getFilename());

}

void CMapDB::delKnownMap(const QStringList& keys)
{
    QString key;
    foreach(key, keys) {
        knownMaps.remove(key);
    }

    emit sigChanged();
}


void CMapDB::loadGPX(CGpx& gpx)
{
}


void CMapDB::saveGPX(CGpx& gpx)
{
}


void CMapDB::loadQLB(CQlb& qlb)
{
}


void CMapDB::saveQLB(CQlb& qlb)
{
}


void CMapDB::upload()
{
}


void CMapDB::download()
{
}


void CMapDB::draw(QPainter& p)
{

    theMap->draw(p);
//     if(visibleMaps.isEmpty()) {
//         defaultMap->draw(p);
//         return;
//     }
//
//     IMap * map;
//     foreach(map,visibleMaps) {
//         map->draw(p);
//     }
}


void CMapDB::editMap()
{
    if(mapedit.isNull()) {
        mapedit = new CMapEditWidget(theMainWindow->getCanvas());
        theMainWindow->setTempWidget(mapedit);
    }
}
