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
#include "CMapTDB.h"
#include "CMapRaster.h"
#include "CMapGeoTiff.h"
#include "CMapDEM.h"
#include "CMainWindow.h"
#include "CMapEditWidget.h"
#include "CMapSearchWidget.h"
#include "GeoMath.h"
#include "CCanvas.h"
#ifdef PLOT_3D
#   include "CMap3DWidget.h"
#endif
#include "CTabWidget.h"
#include "CMapSelectionGarmin.h"
#include "CMapSelectionRaster.h"

#include <QtGui>

CMapDB * CMapDB::m_self = 0;

CMapDB::CMapDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
{
    m_self      = this;
    toolview    = new CMapToolWidget(tb);

    defaultMap = new CMapNoMap(theMainWindow->getCanvas());
    map_t m;
    m.description       = tr("--- No map ---");
    m.key               = "NoMap";
    m.type              = IMap::eRaster;
    knownMaps[m.key]    = m;

    QSettings cfg;
    QString map;
    QStringList maps = cfg.value("maps/knownMaps","").toString().split("|",QString::SkipEmptyParts);
    foreach(map, maps) {
        QFileInfo fi(map);
        QString ext     = fi.suffix().toLower();
        QSettings mapdef(map,QSettings::IniFormat);
        map_t m;
        m.filename      = map;
        if(ext == "tdb") {
            cfg.beginGroup("garmin/maps/alias");
            m.description = cfg.value(fi.fileName(),"").toString();
            cfg.endGroup();
        }
        else {
            m.description = mapdef.value("description/comment","").toString();
        }
        if(m.description.isEmpty()) m.description = QFileInfo(map).fileName();
        m.key           = map;
        m.type          = ext == "qmap" ? IMap::eRaster : ext == "tdb" ? IMap::eVector : IMap::eRaster;
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


IMap& CMapDB::getMap()
{
    return (theMap.isNull() ? *defaultMap : *theMap);
}


IMap& CMapDB::getDEM()
{
    return (demMap.isNull() ? *defaultMap : *demMap);
}


void CMapDB::closeVisibleMaps()
{
    if(!theMap.isNull() && theMap != defaultMap) delete theMap;
    if(!demMap.isNull()) delete demMap;

    theMap = defaultMap;
}


void CMapDB::openMap(const QString& filename, bool asRaster, CCanvas& canvas)
{

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    closeVisibleMaps();

    QSettings cfg;

    map_t map;
    QFileInfo fi(filename);
    QString ext = fi.suffix().toLower();
    if(ext == "qmap") {

        // create map descritor
        QSettings mapdef(filename,QSettings::IniFormat);
        map.filename    = filename;
        map.description = mapdef.value("description/comment","").toString();
        if(map.description.isEmpty()) map.description = fi.fileName();
        map.key         = filename;
        map.type        = IMap::eRaster;

        // create base map
        theMap = new CMapQMAP(map.key, filename,&canvas);

        // add map to known maps
        knownMaps[map.key] = map;

        // store current map filename for next session
        QSettings cfg;
        cfg.setValue("maps/visibleMaps",theMap->getFilename());

    }
    else if(ext == "tdb") {
        CMapTDB * maptdb;

        map.filename    = filename;
        map.key         = filename;
        map.type        = IMap::eVector;

        theMap = maptdb = new CMapTDB(map.key, filename, &canvas);

        map.description = maptdb->getName();
        if(map.description.isEmpty()) map.description = fi.fileName();

        // add map to known maps
        knownMaps[map.key] = map;

        // store current map filename for next session
        cfg.setValue("maps/visibleMaps",filename);
        cfg.beginGroup("garmin/maps/alias");
        cfg.setValue(fi.fileName(),map.description);
        cfg.endGroup();
    }
    else {
        if(asRaster) {
            theMap = new CMapRaster(filename,&canvas);
        }
        else {
            theMap = new CMapGeoTiff(filename,&canvas);
            // store current map filename for next session
            QSettings cfg;
            cfg.setValue("maps/visibleMaps",theMap->getFilename());
        }
    }

    connect(theMap, SIGNAL(sigChanged()),  theMainWindow->getCanvas(), SLOT(update()));

    QString fileDEM = cfg.value(QString("map/dem/%1").arg(theMap->getKey()),"").toString();
    if(!fileDEM.isEmpty()) openDEM(fileDEM);

    emit sigChanged();

    QApplication::restoreOverrideCursor();
}


void CMapDB::openMap(const QString& key)
{
    if(!knownMaps.contains(key)) return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    closeVisibleMaps();

    QString filename = knownMaps[key].filename;
    QFileInfo fi(filename);
    QString ext = fi.suffix().toLower();

    if(ext == "qmap") {
        // create base map
        if(filename.isEmpty()) {
            theMap = defaultMap;
        }
        else {
            theMap = new CMapQMAP(key,filename,theMainWindow->getCanvas());
        }


    }
    else if(ext == "tdb") {
        theMap = new CMapTDB(key,filename,theMainWindow->getCanvas());
    }
    connect(theMap, SIGNAL(sigChanged()), theMainWindow->getCanvas(), SLOT(update()));

    // store current map filename for next session
    QSettings cfg;
    cfg.setValue("maps/visibleMaps",filename);

    QString fileDEM = cfg.value(QString("map/dem/%1").arg(theMap->getKey()),"").toString();
    if(!fileDEM.isEmpty()) openDEM(fileDEM);

    emit sigChanged();
    QApplication::restoreOverrideCursor();
}

IMap * CMapDB::createMap(const QString& key)
{
    if(!knownMaps.contains(key)) return 0;
    const map_t& mapdesc = knownMaps[key];
    if(mapdesc.type != IMap::eVector){
        QMessageBox::critical(0, tr("Error..."), tr("Only vector maps are valid overlays."), QMessageBox::Abort, QMessageBox::Abort);
        return 0;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    IMap * map = 0;
    QString filename = mapdesc.filename;
    QFileInfo fi(filename);
    QString ext = fi.suffix().toLower();

    if(ext == "tdb") {
        map = new CMapTDB(key, filename);
    }
    QApplication::restoreOverrideCursor();

    return map;
}

void CMapDB::openDEM(const QString& filename)
{
    QSettings cfg;

    if(!demMap.isNull()){
        delete demMap;
    }

    try{
        CMapDEM * dem;
        demMap = dem = new CMapDEM(filename, theMainWindow->getCanvas());
        theMap->registerDEM(*dem);
    }
    catch(const QString& msg){
        cfg.setValue(QString("map/dem/%1").arg(theMap->getKey()), "");
        QMessageBox:: critical(0,tr("Error..."), msg, QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    cfg.setValue(QString("map/dem/%1").arg(theMap->getKey()), filename);

    emit sigChanged();
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
        delete selectedMaps.take(key);
    }

    emit sigChanged();
}


void CMapDB::selSelectedMap(const QString& key)
{
    if(!selectedMaps.contains(key)) return;

    IMapSelection * ms = selectedMaps[key];
    if(mapsearch && (ms->type == IMapSelection::eRaster)) mapsearch->setArea((CMapSelectionRaster&)*ms);
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


void CMapDB::draw(QPainter& p, const QRect& rect)
{
    if(theMap.isNull()) {
        defaultMap->draw(p);
        return;
    }
    theMap->draw(p);

    if(!demMap.isNull()) {
        demMap->draw(p);
    }

    if(tabbar-> currentWidget() != toolview) {
        return;
    }

    QMap<QString,IMapSelection*>::iterator ms = selectedMaps.begin();
    while(ms != selectedMaps.end()) {
        (*ms)->draw(p, rect);
        ++ms;
    }
}


void CMapDB::editMap()
{
    if(mapedit.isNull()) {
        mapedit = new CMapEditWidget(theMainWindow->getCanvas());
        theMainWindow->setTempWidget(mapedit);
    }
}


#ifdef PLOT_3D
void CMapDB::show3DMap(bool show)
{
    if(map3DWidget.isNull() && show) {
        map3DWidget = new CMap3DWidget(theMainWindow->getCanvas());
        theMainWindow->getCanvasTab()->addTab(map3DWidget, tr("Map 3D..."));
    }
    else if(!map3DWidget.isNull() && !show) {
        delete map3DWidget;
    }
}
#endif

void CMapDB::searchMap()
{
    if(mapsearch.isNull()) {
        mapsearch = new CMapSearchWidget(theMainWindow->getCanvas());
        theMainWindow->setTempWidget(mapsearch);
    }
}


void CMapDB::select(const QRect& rect)
{
    QString mapkey = theMap->getKey();
    if(mapkey.isEmpty()) {
        QMessageBox::information(0,tr("Sorry..."), tr("You can't select subareas from single file maps."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if(theMap->maptype == IMap::eRaster){
        CMapSelectionRaster * ms = new CMapSelectionRaster(this);
        ms->mapkey       = mapkey;
        ms->description  = knownMaps[mapkey].description;

        theMap->select(*ms, rect);

        selectedMaps[ms->key] = ms;

        if(mapsearch) mapsearch->setArea(*ms);
        emit sigChanged();
    }
    else if(theMap->maptype == IMap::eVector){
        IMapSelection * ms = 0;
        if(selectedMaps.contains("gmapsupp")){
            ms = selectedMaps["gmapsupp"];
        }
        else{
            ms = new CMapSelectionGarmin(this);
        }
        ms->key          = "gmapsupp";
        ms->mapkey       = mapkey;
        ms->description  = "Garmin";
        theMap->select(*ms, rect);

        selectedMaps[ms->key] = ms;
        emit sigChanged();
    }

}
