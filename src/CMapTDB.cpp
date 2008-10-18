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
#include "GeoMath.h"

#include <QtGui>
#include <algorithm>


#define MAX_IDX_ZOOM 35
#define MIN_IDX_ZOOM 0
#undef DEBUG_SHOW_SECTION_BORDERS
#define DEBUG_SHOW_MAPLEVELS

CMapTDB::scale_t CMapTDB::scales[] =
{
    {QString("7000 km"), 70000.0, 8}        //0
    ,{QString("5000 km"), 50000.0, 8}       //1
    ,{QString("3000 km"), 30000.0, 9}       //2
    ,{QString("2000 km"), 20000.0, 9}       //3
    ,{QString("1500 km"), 15000.0, 10}      //4
    ,{QString("1000 km"), 10000.0, 10}      //5
    ,{QString("700 km"), 7000.0, 11}        //6
    ,{QString("500 km"), 5000.0, 11}        //7
    ,{QString("300 km"), 3000.0, 13}        //8
    ,{QString("200 km"), 2000.0, 13}        //9
    ,{QString("150 km"), 1500.0, 13}        //10
    ,{QString("100 km"), 1000.0, 14}        //11
    ,{QString("70 km"), 700.0, 15}          //12
    ,{QString("50 km"), 500.0, 16}          //13
    ,{QString("30 km"), 300.0, 16}          //14
    ,{QString("20 km"), 200.0, 17}          //15
    ,{QString("15 km"), 150.0, 17}          //16
    ,{QString("10 km"), 100.0, 18}          //17
    ,{QString("7 km"), 70.0, 18}            //18
    ,{QString("5 km"), 50.0, 19}            //19
    ,{QString("3 km"), 30.0, 19}            //20
    ,{QString("2 km"), 20.0, 20}            //21
    ,{QString("1.5 km"), 15.0, 22}          //22
    ,{QString("1 km"), 10.0, 24}            //23
    ,{QString("700 m"), 7.0, 24}            //24
    ,{QString("500 m"), 5.0, 24}            //25
    ,{QString("300 m"), 3.0, 24}            //26
    ,{QString("200 m"), 2.0, 24}            //27
    ,{QString("150 m"), 1.5, 24}            //28
    ,{QString("100 m"), 1.0, 24}            //29
    ,{QString("70 m"), 0.7, 24}             //30
    ,{QString("50 m"), 0.5, 24}             //31
    ,{QString("30 m"), 0.3, 24}             //32
    ,{QString("20 m"), 0.2, 24}             //33
    ,{QString("15 m"), 0.1, 24}             //34
    ,{QString("10 m"), 0.15, 24}            //35
};


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
, needRedraw(true)
, zoomFactor(0)
{
    IMap& map   = CMapDB::self().getMap();
    pjsrc       = pj_init_plus(map.getProjection());

    qDebug() << "pjsrc:\t" << pj_get_def(pjsrc,0);
    qDebug() << "pjtar:\t" << pj_get_def(pjtar,0);

    readTDB(filename);
    processPrimaryMapData();

    QSettings cfg;
    cfg.beginGroup("garmin/maps");
    cfg.beginGroup(name);
    QString pos = cfg.value("topleft","").toString();
    zoomidx     = cfg.value("zoomidx",11).toInt();
    cfg.endGroup();
    cfg.endGroup();

    float u = 0;
    float v = 0;
    GPS_Math_Str_To_Deg(pos, u, v);
    topLeft.u = u * DEG_TO_RAD;
    topLeft.v = v * DEG_TO_RAD;

    zoom(zoomidx);

    qDebug() << "CMapTDB::CMapTDB()";
}

CMapTDB::~CMapTDB()
{
    QString pos;
    QSettings cfg;
    cfg.beginGroup("garmin/maps");
    cfg.beginGroup(name);
    GPS_Math_Deg_To_Str(topLeft.u * RAD_TO_DEG, topLeft.v * RAD_TO_DEG, pos);
    pos = pos.replace("\260","");
    cfg.setValue("topleft",pos);
    cfg.setValue("zoomidx",zoomidx);
    cfg.endGroup();
    cfg.endGroup();

    qDebug() << "CMapTDB::~CMapTDB()";
}

void CMapTDB::readTDB(const QString& filename)
{
#ifdef HAVE_BIGENDIAN
    QMessageBox::warning(0,tr("No big endian.."),tr("*tdb import has not been ported to big endian architectures, yet."),QMessageBox::Abort,QMessageBox::Abort);
    return;
#endif
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
                double n        = GARMIN_RAD((p->north >> 8) & 0x00FFFFFF);
                double e        = GARMIN_RAD((p->east >> 8)  & 0x00FFFFFF);
                double s        = GARMIN_RAD((p->south >> 8) & 0x00FFFFFF);
                double w        = GARMIN_RAD((p->west >> 8)  & 0x00FFFFFF);

                if(north < n) north = n;
                if(east  < e) east  = e;
                if(south > s) south = s;
                if(west  > w) west  = w;

                QSettings cfg;
                cfg.beginGroup("garmin/maps");
                cfg.beginGroup(name);
                basemap = cfg.value("basemap","").toString();
                mapkey  = cfg.value("key","").toString();
                cfg.endGroup();
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

                    cfg.beginGroup("garmin/maps");
                    cfg.beginGroup(name);
                    cfg.setValue("basemap",filename);
                    cfg.endGroup();
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

                tile.north  = GARMIN_RAD((p->north >> 8) & 0x00FFFFFF);
                tile.east   = GARMIN_RAD((p->east >> 8)  & 0x00FFFFFF);
                tile.south  = GARMIN_RAD((p->south >> 8) & 0x00FFFFFF);
                tile.west   = GARMIN_RAD((p->west >> 8)  & 0x00FFFFFF);
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
                    cfg.beginGroup("garmin/maps");
                    cfg.beginGroup(name);
                    cfg.setValue("key",mapkey);
                    cfg.endGroup();
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
    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    u = pt.u + u * +1.0 * zoomFactor;
    v = pt.v + v * -1.0 * zoomFactor;
}

void CMapTDB::convertM2Pt(double& u, double& v)
{
    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    u = (u - pt.u) / (+1.0 * zoomFactor);
    v = (v - pt.v) / (-1.0 * zoomFactor);
}

void CMapTDB::convertM2Pt(double* u, double* v, int n)
{
    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    for(int i = 0; i < n; ++i, ++u, ++v){
        *u = (*u - pt.u) / (+1.0 * zoomFactor);
        *v = (*v - pt.v) / (-1.0 * zoomFactor);
    }
};

void CMapTDB::move(const QPoint& old, const QPoint& next)
{
    XY p2 = topLeft;
    convertRad2Pt(p2.u, p2.v);

    // move top left point by difference
    p2.u += old.x() - next.x();
    p2.v += old.y() - next.y();

    // convert back to new top left geo coordinate
    convertPt2Rad(p2.u, p2.v);
    topLeft = p2;

    needsRedraw = true;
    emit sigChanged();
}

void CMapTDB::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

    needsRedraw = true;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    zoomidx += zoomIn ? +1 : -1;
    zoom(zoomidx);

    // convert geo. coordinates back to point
    convertRad2Pt(p1.u, p1.v);

    XY p2 = topLeft;
    convertRad2Pt(p2.u, p2.v);

    // move top left point by difference point befor and after zoom
    p2.u += p1.u - p0.x();
    p2.v += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2Rad(p2.u, p2.v);
    topLeft = p2;

    emit sigChanged();

    qDebug() << "maplevel" /*<< mapLevelMap << "(" << mapLevelOvl << ")"*/ << "bits" << scales[zoomidx].bits;
}

void CMapTDB::zoom(double lon1, double lat1, double lon2, double lat2)
{
    double u[3];
    double v[3];
    double dU, dV;

    u[0] = lon1;
    v[0] = lat1;
    u[1] = lon2;
    v[1] = lat1;
    u[2] = lon1;
    v[2] = lat2;

    pj_transform(pjtar, pjsrc,3,0,u,v,0);
    dU = u[1] - u[0];
    dV = v[2] - v[0];

    for(int i = MAX_IDX_ZOOM; i >= MIN_IDX_ZOOM; --i){

        double z    = scales[i].scale;
        double pxU  = dU / (+1.0 * z);
        double pxV  = dV / (-1.0 * z);

        if((pxU < size.width()) && (pxV < size.height())) {
            zoomFactor  = z;
            zoomidx     = i;
            double u = lon1 + (lon2 - lon1)/2;
            double v = lat1 + (lat2 - lat1)/2;
            convertRad2Pt(u,v);
            move(QPoint(u,v), rect.center());
            return;
        }
    }
}

void CMapTDB::zoom(qint32& level)
{
    needsRedraw = true;

    zoomidx = level;
    if(zoomidx < MIN_IDX_ZOOM) zoomidx = MIN_IDX_ZOOM;
    if(zoomidx > MAX_IDX_ZOOM) zoomidx = MAX_IDX_ZOOM;
    zoomFactor = scales[zoomidx].scale;

    qDebug() << scales[zoomidx].bits << scales[zoomidx].scale << scales[zoomidx].label;

    emit sigChanged();
}

void CMapTDB::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = west;
    lat1 = north;
    lon2 = east;
    lat2 = south;
}

void CMapTDB::draw(QPainter& p)
{
    bottomRight.u = size.width();
    bottomRight.v = size.height();
    convertPt2Rad(bottomRight.u, bottomRight.v);

    if(needsRedraw){
        draw();
    }
    p.drawImage(0,0,buffer);
}

void CMapTDB::draw()
{
    buffer.fill(Qt::white);
    QPainter p(&buffer);

    quint8 bits = scales[zoomidx].bits;
    QVector<map_level_t>::const_iterator maplevel = maplevels.end();
    do{
        --maplevel;
        if(bits >= maplevel->bits) break;
    } while(maplevel != maplevels.begin());

    QRectF viewport(QPointF(topLeft.u, topLeft.v), QPointF(bottomRight.u, bottomRight.v));

    if(maplevel->useBaseMap){
        // draw basemap
        baseimg->draw(p, maplevel->level, zoomFactor, viewport);
    }
    else{
        // draw tiles
    }
}


