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

#include "CMapDB.h"
#include "CMapToolWidget.h"
#include "CMapQMAP.h"
#include "CMapTDB.h"
#include "CMapRaster.h"
#include "CMapGeoTiff.h"
#include "CMapJnx.h"
#include "CMapDEM.h"
#include "CMapOSM.h"
#include "CMainWindow.h"
#include "CMapEditWidget.h"
#include "CMapSearchWidget.h"
#include "GeoMath.h"
#include "CCanvas.h"
#ifdef PLOT_3D
#include "CMap3D.h"
#endif
#include "CTabWidget.h"
#include "CMapSelectionGarmin.h"
#include "CMapSelectionRaster.h"
#include "CResources.h"
#include "IDevice.h"
#ifdef WMS_CLIENT
#include "CMapWMS.h"
#endif
#include "CQlb.h"

#include <QtGui>
#include <QtXml/QDomDocument>

#include <gdal_priv.h>

CMapDB * CMapDB::m_self = 0;

CMapDB::CMapDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
{
    m_self      = this;
    CMapToolWidget * tw = new CMapToolWidget(tb);
    toolview    = tw;

    connect(tw, SIGNAL(sigChanged()), SIGNAL(sigChanged()));

    defaultMap = new CMapNoMap(theMainWindow->getCanvas());
    map_t m;
    m.description       = tr("--- No map ---");
    m.key               = "NoMap";
    m.type              = IMap::eRaster;
    knownMaps[m.key]    = m;

    m.description       = tr("--- OSM ---");
    m.key               = "OSMTileServer";
    m.type              = IMap::eTile;
    knownMaps[m.key]    = m;

    theMap = defaultMap;

    QSettings cfg;
    QString map;
    QStringList maps = cfg.value("maps/knownMaps","").toString().split("|",QString::SkipEmptyParts);
    foreach(map, maps)
    {
        QFileInfo fi(map);
        QString ext     = fi.suffix().toLower();
        QSettings mapdef(map,QSettings::IniFormat);
        map_t m;
        m.filename      = map;
        if(ext == "tdb")
        {
            cfg.beginGroup("garmin/maps/alias");
            m.description = cfg.value(map,"").toString();
            cfg.endGroup();
        }
        else if(ext == "xml")
        {
            QFile file(map);
            file.open(QIODevice::ReadOnly);
            QDomDocument dom;
            dom.setContent(&file, false);
            m.description = dom.firstChildElement("GDAL_WMS").firstChildElement("Service").firstChildElement("Title").text();
            file.close();
            if(m.description.isEmpty()) m.description = fi.fileName();
        }
        else
        {
            m.description = mapdef.value("description/comment","").toString();
        }
        if(m.description.isEmpty()) m.description = QFileInfo(map).fileName();
        m.key           = map;
        m.type          = ext == "qmap" ? IMap::eRaster : ext == "tdb" ? IMap::eGarmin : IMap::eRaster;
        knownMaps[m.key] = m;
    }

    maps = cfg.value("maps/visibleMaps","").toString().split("|",QString::SkipEmptyParts);
    foreach(map, maps)
    {
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
    foreach(map,knownMaps)
    {
        maps += map.filename + "|";
    }
    cfg.setValue("maps/knownMaps",maps);

    GDALDestroyDriverManager();
}


#ifdef PLOT_3D
CMap3D * CMapDB::getMap3D()
{
    return map3D;
}
#endif

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
    if(ext == "qmap")
    {

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
    else if(ext == "tdb")
    {
        CMapTDB * maptdb;

        map.filename    = filename;
        map.key         = filename;
        map.type        = IMap::eGarmin;

        theMap = maptdb = new CMapTDB(map.key, filename, &canvas);

        map.description = maptdb->getName();
        if(map.description.isEmpty()) map.description = fi.fileName();

        // add map to known maps
        knownMaps[map.key] = map;

        // store current map filename for next session
        cfg.setValue("maps/visibleMaps",filename);
        cfg.beginGroup("garmin/maps/alias");
        cfg.setValue(map.filename, map.description);
        cfg.endGroup();
    }
#ifdef HAS_JNX
    else if(ext == "jnx")
    {

        CMapJnx * mapjnx;

        map.filename    = filename;
        map.key         = filename;
        map.type        = IMap::eRaster;

        theMap = mapjnx = new CMapJnx(map.key, filename, &canvas);

        map.description = mapjnx->getName();
        if(map.description.isEmpty()) map.description = fi.fileName();

        // add map to known maps
        knownMaps[map.key] = map;

        // store current map filename for next session
        cfg.setValue("maps/visibleMaps",filename);
        cfg.beginGroup("garmin/maps/alias");
        cfg.setValue(map.filename, map.description);
        cfg.endGroup();
    }
#endif // HAS_JNX
#ifdef WMS_CLIENT
    else if(ext == "xml" )
    {
        map.filename    = filename;
        map.key         = filename;
        map.type        = IMap::eRaster;

        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QDomDocument dom;
        dom.setContent(&file, false);
        map.description = dom.firstChildElement("GDAL_WMS").firstChildElement("Service").firstChildElement("Title").text();
        file.close();

        if(map.description.isEmpty()) map.description = fi.fileName();
        theMap = new CMapWMS(map.key,filename,theMainWindow->getCanvas());

        // add map to known maps
        knownMaps[map.key] = map;

        // store current map filename for next session
        QSettings cfg;
        cfg.setValue("maps/visibleMaps",theMap->getFilename());
    }
#endif
    else if(filename == "OSMTileServer")
    {
        theMap = new CMapOSM(theMainWindow->getCanvas());

        // store current map filename for next session
        QSettings cfg;
        cfg.setValue("maps/visibleMaps",filename);

    }
    else
    {
        if(asRaster)
        {
            theMap = new CMapRaster(filename,&canvas);
        }
        else
        {
            theMap = new CMapGeoTiff(filename,&canvas);
            // store current map filename for next session
            QSettings cfg;
            cfg.setValue("maps/visibleMaps",theMap->getFilename());
        }
    }

    connect(theMap, SIGNAL(sigChanged()),  theMainWindow->getCanvas(), SLOT(update()));

    QString fileDEM = cfg.value(QString("map/dem/%1").arg(theMap->getKey()),"").toString();
    if(!fileDEM.isEmpty()) openDEM(fileDEM);

#ifdef PLOT_3D_NEW
//    CMap3D * map3D = new CMap3D(theMap, theMainWindow->getCanvas());
//    theMainWindow->getCanvasTab()->addTab(map3D, tr("Map 3D..."));
#endif

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

    if(ext == "qmap")
    {
        // create base map
        if(filename.isEmpty())
        {
            theMap = defaultMap;
        }
        else
        {
            theMap = new CMapQMAP(key,filename,theMainWindow->getCanvas());
        }

    }
    else if(ext == "tdb")
    {
        theMap = new CMapTDB(key,filename,theMainWindow->getCanvas());
    }
#ifdef HAS_JNX
    else if(ext == "jnx")
    {
        theMap = new CMapJnx(key,filename,theMainWindow->getCanvas());
    }
#endif // HAS_JNX
#ifdef WMS_CLIENT
    else if(ext == "xml" )
    {
        theMap = new CMapWMS(key,filename,theMainWindow->getCanvas());
    }
#endif
    else if(key == "OSMTileServer")
    {
        theMap = new CMapOSM(theMainWindow->getCanvas());
        filename = key;
    }

    connect(theMap, SIGNAL(sigChanged()), theMainWindow->getCanvas(), SLOT(update()));

    // store current map filename for next session
    QSettings cfg;
    cfg.setValue("maps/visibleMaps",filename);

    QString fileDEM = cfg.value(QString("map/dem/%1").arg(theMap->getKey()),"").toString();
    if(!fileDEM.isEmpty()) openDEM(fileDEM);

    double lon1, lon2, lat1, lat2;
    theMap->dimensions(lon1, lat1, lon2, lat2);
    if(((lon1 < IMap::midU) && (IMap::midU < lon2)) && ((lat2 < IMap::midV) && (IMap::midV < lat1)) && ((IMap::midU != 0) && (IMap::midV != 0)))
    {
        double midU = IMap::midU;
        double midV = IMap::midV;
        theMap->convertRad2Pt(midU, midV);
        theMap->move(QPoint(midU, midV), theMainWindow->getCanvas()->rect().center());
    }

#ifdef PLOT_3D_NEW
//    CMap3D * map3D = new CMap3D(theMap, theMainWindow->getCanvas());
//    theMainWindow->getCanvasTab()->addTab(map3D, tr("Map 3D..."));
#endif

    emit sigChanged();
    QApplication::restoreOverrideCursor();
}


IMap * CMapDB::createMap(const QString& key)
{
    if(!knownMaps.contains(key)) return 0;
    const map_t& mapdesc = knownMaps[key];
    if(mapdesc.type != IMap::eGarmin)
    {
        QMessageBox::critical(0, tr("Error..."), tr("Only vector maps are valid overlays."), QMessageBox::Abort, QMessageBox::Abort);
        return 0;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    IMap * map = 0;
    QString filename = mapdesc.filename;
    QFileInfo fi(filename);
    QString ext = fi.suffix().toLower();

    if(ext == "tdb")
    {
        map = new CMapTDB(key, filename);
    }
    QApplication::restoreOverrideCursor();

    return map;
}


void CMapDB::openDEM(const QString& filename)
{
    QSettings cfg;

    if(!demMap.isNull())
    {
        delete demMap;
    }

    try
    {
        CMapDEM * dem;
        dem = new CMapDEM(filename, theMainWindow->getCanvas());
        if (dem->loaded())
            demMap = dem, theMap->registerDEM(*dem);
        else
            delete dem;
    }
    catch(const QString& msg)
    {
        cfg.setValue(QString("map/dem/%1").arg(theMap->getKey()), "");
        cfg.setValue(QString("map/dem/%1/ignoreWarning").arg(theMap->getKey()), false);
        return;
    }

    cfg.setValue(QString("map/dem/%1").arg(theMap->getKey()), filename);
    if(filename.isEmpty())
    {
        cfg.setValue(QString("map/dem/%1/ignoreWarning").arg(theMap->getKey()), false);
    }

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
    foreach(key, keys)
    {
        map_t& map = knownMaps[key];
        if(map.type == IMap::eGarmin)
        {
            QSettings cfg;
            cfg.beginGroup("garmin/maps");
            cfg.beginGroup("alias");
            QString name = cfg.value(key,key).toString();
            cfg.endGroup();
            cfg.remove(name);
            cfg.endGroup();
            cfg.sync();
        }
        knownMaps.remove(key);
    }

    emit sigChanged();
}


void CMapDB::delSelectedMap(const QStringList& keys)
{
    QString key;
    foreach(key, keys)
    {
        delSelectedMap(key,true);
    }

    emit sigChanged();
}

void CMapDB::delSelectedMap(const QString& key, bool silent)
{
    if(selectedMaps.contains(key))
    {
        delete selectedMaps.take(key);
        if(!silent)
        {
            emit sigChanged();
        }
    }
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


void CMapDB::saveGPX(CGpx& gpx, const QStringList& keys)
{
}


QDataStream& CMapDB::operator<<(QDataStream& s)
{
    qint32 type;
    quint32 timestamp;
    QString key;
    QString mapkey;
    QString name;
    QString comment;
    QString description;


    double lon1;             ///< top left longitude [rad]
    double lat1;             ///< top left latitude [rad]
    double lon2;             ///< bottom right longitude [rad]
    double lat2;             ///< bottom right latitude [rad]

    QIODevice * dev = s.device();
    qint64      pos = dev->pos();

    char magic[9];
    s.readRawData(magic,9);

    if(strncmp(magic,"QLMapSel",9))
    {
        dev->seek(pos);
        return s;
    }

    QList<IMapSelection::sel_head_entry_t> entries;
    while(1)
    {
        IMapSelection::sel_head_entry_t entry;
        s >> entry.type >> entry.offset;
        entries << entry;
        if(entry.type == CWpt::eEnd) break;
    }

    QList<IMapSelection::sel_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end())
    {
        qint64 o = pos + entry->offset;
        dev->seek(o);
        s >> entry->data;

        switch(entry->type)
        {
            case IMapSelection::eHeadBase:
            {

                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);

                s1 >> type;
                s1 >> key;
                s1 >> mapkey;
                s1 >> timestamp;
                s1 >> name;
                s1 >> comment;
                s1 >> description;
                s1 >> lon1;             ///< top left longitude [rad]
                s1 >> lat1;             ///< top left latitude [rad]
                s1 >> lon2;             ///< bottom right longitude [rad]
                s1 >> lat2;             ///< bottom right latitude [rad]
                break;
            }

            case IMapSelection::eHeadRaster:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);

                CMapSelectionRaster * ms = new CMapSelectionRaster(this);
                ms->setKey(key);
                ms->mapkey = mapkey;
                ms->setTimestamp(timestamp);
                ms->setName(name);
                ms->setComment(comment);
                ms->setDescription(description);
                ms->lon1 = lon1;
                ms->lat1 = lat1;
                ms->lon2 = lon2;
                ms->lat2 = lat2;

                s1 >> ms->selTiles ;

                selectedMaps[ms->getKey()] = ms;

                break;
            }

            case IMapSelection::eHeadGarmin:
            {
                int nMaps, nTiles, m, t;

                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);

                CMapSelectionGarmin * ms = new CMapSelectionGarmin(this);
                ms->setKey(key);
                ms->mapkey = mapkey;
                ms->setTimestamp(timestamp);
                ms->setName(name);
                ms->setComment(comment);
                ms->setDescription(description);
                ms->lon1 = lon1;
                ms->lat1 = lat1;
                ms->lon2 = lon2;
                ms->lat2 = lat2;

                s1 >> nMaps;
                for(m = 0; m < nMaps; m++)
                {
                    QString key;
                    s1 >> key;
                    CMapSelectionGarmin::map_t map;
                    s1 >> map.unlockKey;
                    s1 >> map.name;
                    s1 >> map.typfile;
                    s1 >> map.mdrfile;
                    s1 >> map.fid;
                    s1 >> map.pid;

                    s1 >> nTiles;
                    for(t = 0; t < nTiles; t++)
                    {
                        QString key;
                        s1 >> key;
                        CMapSelectionGarmin::tile_t tile;
                        s1 >> tile.id;
                        s1 >> tile.name;
                        s1 >> tile.filename;
                        s1 >> tile.u;
                        s1 >> tile.v;
                        s1 >> tile.memSize;
                        s1 >> tile.area;
                        s1 >> tile.fid;
                        s1 >> tile.pid;

                        map.tiles[key] = tile;
                    }

                    ms->maps[key] = map;
                }

                QString key;
                foreach(key, selectedMaps.keys())
                {
                    IMapSelection * mapSel = selectedMaps[key];
                    if(mapSel->type == IMapSelection::eGarmin)
                    {
                        delete selectedMaps.take(key);
                    }
                }

                selectedMaps[ms->getKey()] = ms;
                break;
            }
            default:;
        }

        ++entry;
    }
    return s;
}

void CMapDB::loadQLB(CQlb& qlb, bool newKey)
{
    QDataStream stream(&qlb.mapsels(),QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_5);

    while(!stream.atEnd())
    {
        *this << stream;
    }
    if(selectedMaps.size())
    {
        emitSigChanged();
    }
}


void CMapDB::saveQLB(CQlb& qlb)
{
    QMap<QString, IMapSelection*>::iterator sel = selectedMaps.begin();
    while(sel != selectedMaps.end())
    {
        qlb << *(*sel);
        ++sel;
    }
}


void CMapDB::upload(const QStringList& keys)
{
    if(selectedMaps.isEmpty()) return;

    IDevice * dev = CResources::self().device();
    if(dev)
    {
        QList<IMapSelection*> tmpms = selectedMaps.values();
        dev->uploadMap(tmpms);
    }
}


void CMapDB::download()
{
}


void CMapDB::draw(QPainter& p, const QRect& rect, bool& needsRedraw)
{
    if(theMap.isNull())
    {
        defaultMap->draw(p);
        return;
    }
    needsRedraw = theMap->getNeedsRedraw();
    theMap->draw(p);

    if(!demMap.isNull())
    {
        demMap->draw(p);
    }

    if(tabbar-> currentWidget() != toolview)
    {
        return;
    }

    QMap<QString,IMapSelection*>::iterator ms = selectedMaps.begin();
    while(ms != selectedMaps.end())
    {
        (*ms)->draw(p, rect);
        ++ms;
    }
}


void CMapDB::editMap()
{
    if(mapedit.isNull())
    {
        mapedit = new CMapEditWidget(theMainWindow->getCanvas());
        theMainWindow->setTempWidget(mapedit, tr("Edit Map"));
    }
}


#ifdef PLOT_3D
void CMapDB::show3DMap(bool show)
{
    if(map3D.isNull() && show)
    {
        if(!theMap.isNull())
        {
            map3D = new CMap3D(theMap, theMainWindow->getCanvas());
            theMainWindow->getCanvasTab()->addTab(map3D, tr("Map 3D..."));
        }
    }
    else if(!map3D.isNull() && !show)
    {
        map3D->deleteLater();
    }
}
#endif

void CMapDB::searchMap()
{
    if(mapsearch.isNull())
    {
        mapsearch = new CMapSearchWidget(theMainWindow->getCanvas());
        theMainWindow->setTempWidget(mapsearch, tr("Search Map"));
    }
    else
    {
        mapsearch->deleteLater();
    }
}


void CMapDB::select(const QRect& rect, const QMap< QPair<int,int>, bool>& selTiles)
{
    QString mapkey = theMap->getKey();
    if(mapkey.isEmpty())
    {
        QMessageBox::information(0,tr("Sorry..."), tr("You can't select subareas from single file maps. Create a collection with F1->F6."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if(theMap->maptype == IMap::eRaster || theMap->maptype == IMap::eTile)
    {
        CMapSelectionRaster * ms = new CMapSelectionRaster(this);
        ms->mapkey       = mapkey;
        ms->selTiles     = selTiles;
        ms->setName(knownMaps[mapkey].description);

        try
        {
            theMap->select(*ms, rect);

            selectedMaps[ms->getKey()] = ms;

            if(ms->isEmpty())
            {
                delete selectedMaps.take(ms->getKey());
            }
            else if(mapsearch)
            {
                mapsearch->setArea(*ms);
            }

            emit sigChanged();
        }
        catch(const QString& msg)
        {
            delete ms;
            QMessageBox::critical(0,tr("Error..."), msg, QMessageBox::Abort,QMessageBox::Abort);
        }

    }
    else if(theMap->maptype == IMap::eGarmin)
    {
        bool isEdit = false;
        IMapSelection * ms = 0;
        IMapSelection * mapSel;
        foreach(mapSel, selectedMaps)
        {
            if(mapSel->type == IMapSelection::eGarmin)
            {
                ms     = mapSel;
                isEdit = true;
                break;
            }
        }

        if(ms == 0)
        {
            ms = new CMapSelectionGarmin(this);
        }


        ms->mapkey       = mapkey;
        ms->setDescription("Garmin - gmapsupp.img");
        theMap->select(*ms, rect);

        selectedMaps[ms->getKey()] = ms;

        if(ms->isEmpty())
        {
            delete selectedMaps.take(ms->getKey());
            ms = 0;
        }

        if(ms && isEdit)
        {
            emit sigModified(ms->getKey());
        }
        emit sigChanged();
    }
}


IMapSelection * CMapDB::getSelectedMap(double lon, double lat)
{
    IMap& map = getMap();

    if(map.maptype != IMap::eRaster)
    {
        return 0;
    }

    if(tabbar->currentWidget() != toolview)
    {
        return 0;
    }


    QString mapkey = map.getKey();
    IMapSelection * mapSel = 0;
    foreach(mapSel, selectedMaps)
    {
        if(mapSel->mapkey != mapkey)
        {
            continue;
        }

        QRectF r(QPointF(mapSel->lon1, mapSel->lat1), QPointF(mapSel->lon2, mapSel->lat2));
        if(r.contains(lon, lat))
        {
            return mapSel;
        }
    }

    return 0;
}

IMapSelection * CMapDB::getMapSelectionByKey(const QString& key)
{
    if(selectedMaps.contains(key))
    {
        return selectedMaps[key];
    }

    return 0;
}
