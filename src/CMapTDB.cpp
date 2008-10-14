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
#include "CMapTDB.h"
#include "CMapDB.h"
#include "Garmin.h"
#include "CMapGarminTile.h"

#include <QtGui>
#include <algorithm>

#define DEBUG_SHOW_MAPLEVELS

CMapTDB::CMapTDB(const QString& key, const QString& filename, CCanvas * parent)
: IMap(key,parent)
, filename(filename)
, north(-90.0)
, east(-180.0)
, south(90.0)
, west(180.0)
, encrypted(false)
, baseimg(0)
, isTransparent(false)
{
    IMap& map   = CMapDB::self().getMap();
    pjsrc       = pj_init_plus(map.getProjection());

    qDebug() << "pjsrc:\t" << pj_get_def(pjsrc,0);
    qDebug() << "pjtar:\t" << pj_get_def(pjtar,0);

    readTDB(filename);
    processPrimaryMapData();

    qDebug() << "CMapTDB::CMapTDB()";
}

CMapTDB::~CMapTDB()
{
    qDebug() << "CMapTDB::~CMapTDB()";
}

void CMapTDB::readTDB(const QString& filename)
{
    QByteArray  data;
    QFile       file(filename);
    QFileInfo   finfo(filename);

    file.open(QIODevice::ReadOnly);
    data = file.readAll();
    file.close();

    quint8 * const pRawData = (quint8*)data.data();
    tdb_hdr_t * pRecord     = (tdb_hdr_t*)pRawData;

    quint32 basemapId = 0;

    while((quint8*)pRecord < pRawData + data.size()) {
        switch(pRecord->type) {
            case 0x50:           // product name
            {
                tdb_product_t * p = (tdb_product_t*)pRecord;
                name = (char*)p->name;
            }
            break;

            case 0x42:           // basemap
            {
                tdb_map_t * p   = (tdb_map_t*)pRecord;

                basemapId       = p->id;
                double n        = GARMIN_DEG((p->north >> 8) & 0x00FFFFFF);
                double e        = GARMIN_DEG((p->east >> 8)  & 0x00FFFFFF);
                double s        = GARMIN_DEG((p->south >> 8) & 0x00FFFFFF);
                double w        = GARMIN_DEG((p->west >> 8)  & 0x00FFFFFF);

                if(north < n) north = n;
                if(east  < e) east  = e;
                if(south > s) south = s;
                if(west  > w) west  = w;

                QSettings cfg;
                cfg.beginGroup("garmin/maps/basemap");
                basemap = cfg.value(name,"").toString();
                cfg.endGroup();

                cfg.beginGroup("garmin/maps/key");
                mapkey = cfg.value(name,"").toString();
                cfg.endGroup();

                QFileInfo basemapFileInfo(basemap);
                if(!basemapFileInfo.isFile()) {
                    QString filename = QFileDialog::getOpenFileName( 0, tr("Select Base Map for ") + name
                        ,finfo.dir().path()
                        ,"Map File (*.img)"
                        );
                    if(filename.isEmpty()) {
                        deleteLater();
                        return;
                    }

                    cfg.beginGroup("garmin/maps/basemap");
                    cfg.setValue(name,filename);
                    cfg.endGroup();

                    basemap = filename;
                }

                area = QRect(QPoint(west, north), QPoint(east, south));

            }
            break;

            case 0x4C:           // map tiles
            {
                if(encrypted) break;

                tdb_map_t * p = (tdb_map_t*)pRecord;
                if(p->id == basemapId) break;

                QString tilename = QString::fromLatin1(p->name);
                // produce a unique key form the tile name and it's ID. Some tiles
                // might have the same name but never the same ID
                QString key = QString("%1 (%2)").arg(tilename).arg(p->id,8,10,QChar('0'));

                tile_t& tile    = tiles[key];
                tile.id         = p->id;
                tile.key        = key;
                tile.name       = tilename;
                tile.file.sprintf("%08i.img",p->id);
                tile.file = finfo.dir().filePath(tile.file);

                tile.north  = GARMIN_DEG((p->north >> 8) & 0x00FFFFFF);
                tile.east   = GARMIN_DEG((p->east >> 8)  & 0x00FFFFFF);
                tile.south  = GARMIN_DEG((p->south >> 8) & 0x00FFFFFF);
                tile.west   = GARMIN_DEG((p->west >> 8)  & 0x00FFFFFF);
                tile.area   = QRect(QPoint(tile.west, tile.north), QPoint(tile.east, tile.south));

                tile.memSize = 0;
                tdb_map_size_t * s = (tdb_map_size_t*)(p->name + tilename.size() + 1);

                for(quint16 i=0; i < s->count; ++i) {
                    tile.memSize += s->sizes[i];
                }

                try {
                    tile.img = new CMapGarminTile(this);
                    tile.img->readBasics(tile.file);
                }
                catch(CMapGarminTile::exce_t e){

                    if(e.err == CMapGarminTile::errLock){
                        tiles.clear();
                        encrypted = true;
                    }
                    else{
                        QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Abort,QMessageBox::Abort);
                        deleteLater();
                        return;
                    }

                    if(mapkey.isEmpty()) {
                        QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Abort,QMessageBox::Abort);
                        // help is on the way!!!
                        mapkey = QInputDialog::getText(0,tr("However ...")
                            ,tr("<p><b>However ...</b></p>"
                            "<p>as I can read the basemap, and the information from the *tdb file,<br/>"
                            "I am able to let you select the map tiles for upload. To do this I<br/>"
                            "need the unlock key (25 digits) for this map, as it has to be uploaded<br/>"
                            "to the unit together with the map.</p>"
                            ));
                        // no money, no brother, no sister - no key
                        if(mapkey.isEmpty()) {
                            deleteLater();
                            return;
                        }
                    }
                    QSettings cfg;
                    cfg.beginGroup("garmin/maps/key");
                    cfg.setValue(name,mapkey);
                    cfg.endGroup();
                }

//                 qDebug() << "tile:\t\t" << tile.file;
//                 qDebug() << "name:\t\t" << tile.name;
//                 qDebug() << "dimensions:\t" << "N" << tile.north << "E" << tile.east << "S" << tile.south << "W" << tile.west;
//                 qDebug() << "memsize:\t" << tile.memSize;
            }
            break;

            case 0x44:
            {
                QString str;
                QTextStream out(&str,QIODevice::WriteOnly);

                out << "<h1>" << name << "</h1>" << endl;

                tdb_copyrights_t * p = (tdb_copyrights_t*)pRecord;
                tdb_copyright_t  * c = &p->entry;
                while((void*)c < (void*)((quint8*)p + p->size + 3)) {

                    if(c->type != 0x07) {
                        out << c->str << "<br/>" << endl;
                    }
                    c = (tdb_copyright_t*)((quint8*)c + 4 + strlen(c->str) + 1);
                }

                copyright += str;

//                 qDebug() << "copyright:" << copyright;
            }
            break;
        }

        pRecord = (tdb_hdr_t*)((quint8*)pRecord + pRecord->size + sizeof(tdb_hdr_t));
    }
}

bool CMapTDB::processPrimaryMapData()
{
    try {
        baseimg = new CMapGarminTile(this);
        baseimg->readBasics(basemap);
    }
    catch(CMapGarminTile::exce_t e){
        // no basemap? bad luck!
        QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Ok,QMessageBox::NoButton);
        deleteLater();
        return false;
    }

    qDebug() << "name:\t\t" << name;
    qDebug() << "basemap:\t" << basemap;
    qDebug() << "dimensions:\t" << "N" << north << "E" << east << "S" << south << "W" << west;

    const QMap<QString,CMapGarminTile::subfile_desc_t>& subfiles            = baseimg->getSubFiles();
    QMap<QString,CMapGarminTile::subfile_desc_t>::const_iterator subfile    = subfiles.begin();
    quint8 fewest_map_bits = 0xFF;

    /* Put here so the submap check doesn't do the basemap again. */
    QMap<QString,CMapGarminTile::subfile_desc_t>::const_iterator basemap_subfile;

    /* Find best candidate for basemap. */
    while (subfile != subfiles.end()) {
        QVector<CMapGarminTile::maplevel_t>::const_iterator maplevel = subfile->maplevels.begin();
        /* Skip any upper levels that contain no real data. */
        while (!maplevel->inherited) {
            ++maplevel;
        }
        /* Check for the least detailed map. */
        if (maplevel->bits < fewest_map_bits) {
            fewest_map_bits = maplevel->bits;
            basemap_subfile = subfile;
        }
        ++subfile;
    }

    /* Add all basemap levels to the list. */
    QVector<CMapGarminTile::maplevel_t>::const_iterator maplevel = basemap_subfile->maplevels.begin();
    while(maplevel != basemap_subfile->maplevels.end()) {
        if (!maplevel->inherited) {
            map_level_t ml;
            ml.bits  = maplevel->bits;
            ml.level = maplevel->level;
            ml.useBaseMap = true;
            maplevels << ml;
        }
        ++maplevel;
    }

    if(!tiles.isEmpty()){
        CMapGarminTile * img = tiles.values().first().img;
        const QMap<QString,CMapGarminTile::subfile_desc_t>& subfiles = img->getSubFiles();
        QMap<QString,CMapGarminTile::subfile_desc_t>::const_iterator subfile = subfiles.begin();
        /*
        * Query all subfiles for possible maplevels.
        * Exclude basemap to avoid polution.
        */
        while (subfile != subfiles.end()) {
            QVector<CMapGarminTile::maplevel_t>::const_iterator maplevel = subfile->maplevels.begin();
            /* Skip basemap. */
            if (subfile == basemap_subfile) {
                ++subfile;
                continue;
            }
            while (maplevel != subfile->maplevels.end()) {
                if (!maplevel->inherited) {
                    map_level_t ml;
                    ml.bits  = maplevel->bits;
                    ml.level = maplevel->level;
                    ml.useBaseMap = false;
                    maplevels << ml;
                }
                ++maplevel;
            }
            ++subfile;
        }

        /* Sort all entries, note that stable sort should insure that basemap is preferred when available. */
        qStableSort(maplevels.begin(), maplevels.end(), map_level_t::GreaterThan);
        /* Delete any duplicates for obvious performance reasons. */
        QVector<map_level_t>::iterator where;
        where = std::unique(maplevels.begin(), maplevels.end());
        maplevels.erase(where, maplevels.end());
        isTransparent = img->isTransparent();
    }

#ifdef DEBUG_SHOW_MAPLEVELS
    for(int i=0; i < maplevels.count(); ++i){
        map_level_t& ml = maplevels[i];
        qDebug() << ml.bits << ml.level << ml.useBaseMap;
    }
#endif

    return true;
}

void CMapTDB::convertPt2M(double& u, double& v)
{
}

void CMapTDB::convertM2Pt(double& u, double& v)
{
}

void CMapTDB::move(const QPoint& old, const QPoint& next)
{
}

void CMapTDB::zoom(bool zoomIn, const QPoint& p)
{
}

void CMapTDB::zoom(double lon1, double lat1, double lon2, double lat2)
{
}

void CMapTDB::zoom(qint32& level)
{
}

void CMapTDB::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = DEG_TO_RAD * west;
    lat1 = DEG_TO_RAD * north;
    lon2 = DEG_TO_RAD * east;
    lat2 = DEG_TO_RAD * south;
}
