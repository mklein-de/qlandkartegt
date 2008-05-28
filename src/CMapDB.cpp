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
#include "CMapGeoTiff.h"
#include "CMapDEM.h"
#include "CMainWindow.h"
#include "CMapEditWidget.h"
#include "GeoMath.h"
#include "CCanvas.h"

#include <QtGui>

CMapDB * CMapDB::m_self = 0;

QString CMapDB::mapsel_t::focusedMap;

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
        openMap(map, false, *theMainWindow->getCanvas());
//         QFileInfo fi(map);
//         QString ext = fi.suffix();
//         if(ext == "qmap") {
//             theMap = new CMapQMAP(map,theMainWindow->getCanvas());
//         }
//         else{
//             theMap = new CMapRaster(map,theMainWindow->getCanvas());
//         }
        //TODO: has to be removed for several layers
        break;
    }
    emit sigChanged();

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

void CMapDB::clear()
{
    selectedMaps.clear();
    emit sigChanged();
}

IMap& CMapDB::getMap() {
    return (theMap.isNull() ? *defaultMap : *theMap);
}

IMap& CMapDB::getDEM()
{
    return (demMap.isNull() ? *defaultMap : *demMap);
}

void CMapDB::closeVisibleMaps()
{
    if(!theMap.isNull()) delete theMap;
    if(!demMap.isNull()) delete demMap;
}


void CMapDB::openMap(const QString& filename, bool asRaster, CCanvas& canvas)
{

    closeVisibleMaps();

    map_t map;
    QFileInfo fi(filename);
    QString ext = fi.suffix();
    if(ext == "qmap") {

        // create map descritor
        QSettings mapdef(filename,QSettings::IniFormat);
        map.filename    = filename;
        map.description = mapdef.value("description/comment","").toString();
        if(map.description.isEmpty()) map.description = fi.fileName();
        map.key         = filename;

        // create base map
        theMap = new CMapQMAP(map.key, filename,&canvas);

        // create DEM map if any
        QDir    path        = QFileInfo(filename).absolutePath();
        QString fileDEM     = mapdef.value("DEM/file","").toString();
        QString datum       = mapdef.value("gridshift/datum","").toString();
        QString gridfile    = mapdef.value("gridshift/file","").toString();

        if(!fileDEM.isEmpty()) {
            demMap = new CMapDEM(path.filePath(fileDEM), &canvas, datum, path.filePath(gridfile));
        }

        // add map to known maps
        knownMaps[map.key] = map;

        // store current map filename for next session
        QSettings cfg;
        cfg.setValue("maps/visibleMaps",theMap->getFilename());
    }
    else {
        if(asRaster){
            theMap = new CMapRaster(filename,&canvas);
        }
        else{
            theMap = new CMapGeoTiff(filename,&canvas);
            // store current map filename for next session
            QSettings cfg;
            cfg.setValue("maps/visibleMaps",theMap->getFilename());
        }
    }

    connect(theMap, SIGNAL(sigChanged()), SIGNAL(sigChanged()));
    emit sigChanged();
}


void CMapDB::openMap(const QString& key)
{
    if(!knownMaps.contains(key)) return;

    closeVisibleMaps();

    // create base map
    QString filename = knownMaps[key].filename;
    theMap = new CMapQMAP(key,filename,theMainWindow->getCanvas());

    // create DEM map if any
    QSettings mapdef(filename,QSettings::IniFormat);
    QDir    path        = QFileInfo(filename).absolutePath();
    QString fileDEM     = mapdef.value("DEM/file","").toString();
    QString datum       = mapdef.value("gridshift/datum","").toString();
    QString gridfile    = mapdef.value("gridshift/file","").toString();

    if(!fileDEM.isEmpty()) {
        demMap = new CMapDEM(path.filePath(fileDEM), theMainWindow->getCanvas(), datum, path.filePath(gridfile));
    }

    connect(theMap, SIGNAL(sigChanged()), SIGNAL(sigChanged()));
    // store current map filename for next session
    QSettings cfg;
    cfg.setValue("maps/visibleMaps",theMap->getFilename());

}

void CMapDB::closeMap()
{
    QSettings cfg;
    cfg.setValue("maps/visibleMaps",theMap->getFilename());
    closeVisibleMaps();
}

void CMapDB::delKnownMap(const QStringList& keys)
{
    QString key;
    foreach(key, keys) {
        knownMaps.remove(key);
    }

    emit sigChanged();
}

void CMapDB::delSelectedMap(const QStringList& keys)
{
    QString key;
    foreach(key, keys) {
        selectedMaps.remove(key);
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
    if(theMap.isNull()){
        defaultMap->draw(p);
        return;
    }
    theMap->draw(p);

    if(!demMap.isNull()){
        demMap->draw(p);
    }

    if(tabbar-> currentWidget() != toolview){
        return;
    }

    mapsel_t ms;
    foreach(ms, selectedMaps){


        QString pos1, pos2;

        GPS_Math_Deg_To_Str(ms.lon1 * RAD_TO_DEG, ms.lat1 * RAD_TO_DEG, pos1);
        GPS_Math_Deg_To_Str(ms.lon2 * RAD_TO_DEG, ms.lat2 * RAD_TO_DEG, pos2);

        theMap->convertRad2Pt(ms.lon1, ms.lat1);
        theMap->convertRad2Pt(ms.lon2, ms.lat2);

        p.setBrush(Qt::NoBrush);

        if(ms.focusedMap == ms.key){
            p.setPen(QPen(Qt::red,2));
        }
        else if(ms.mapkey == theMap->getKey()){
            p.setPen(QPen(Qt::darkBlue,2));
        }
        else{
            p.setPen(QPen(Qt::gray,2));
        }

        QRect r(ms.lon1, ms.lat1, ms.lon2 - ms.lon1, ms.lat2 - ms.lat1);
        p.drawRect(r);

        CCanvas::drawText(QString("%1\n%2\n%3").arg(ms.description).arg(pos1).arg(pos2),p,r);
    }
}

void CMapDB::editMap()
{
    if(mapedit.isNull()) {
        mapedit = new CMapEditWidget(theMainWindow->getCanvas());
        theMainWindow->setTempWidget(mapedit);
    }
}

void CMapDB::select(const QRect& rect)
{
    mapsel_t ms;
    ms.mapkey = theMap->getKey();
    if(ms.mapkey.isEmpty()){
        QMessageBox::information(0,tr("Sorry..."), tr("You can't select subareas from single file maps."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }


    ms.description = knownMaps[ms.mapkey].description;

    ms.lon1 = rect.left();
    ms.lat1 = rect.top();
    theMap->convertPt2Rad(ms.lon1, ms.lat1);

    ms.lon2 = rect.right();
    ms.lat2 = rect.bottom();
    theMap->convertPt2Rad(ms.lon2, ms.lat2);

    ms.key = QString("%1%2%3").arg(ms.mapkey).arg(ms.lon1).arg(ms.lat1);

    selectedMaps[ms.key] = ms;
    emit sigChanged();
}

