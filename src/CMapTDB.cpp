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
#include "CMapDEM.h"
#include "Garmin.h"
#include "CGarminTile.h"
#include "GeoMath.h"
#include "CResources.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "IUnit.h"
#include "Platform.h"
#include "CMapSelectionGarmin.h"

#include <QtGui>
#include <algorithm>

#define MAX_IDX_ZOOM 35
#define MIN_IDX_ZOOM 0
#undef DEBUG_SHOW_SECTION_BORDERS
#define DEBUG_SHOW_MAPLEVELS

#define TEXTWIDTH   300

CMapTDB::scale_t CMapTDB::scales[] =
{
    {                            //0
        QString("7000 km"), 70000.0, 8
    }
    ,                            //1
    {
        QString("5000 km"), 50000.0, 8
    }
    ,                            //2
    {
        QString("3000 km"), 30000.0, 9
    }
    ,                            //3
    {
        QString("2000 km"), 20000.0, 9
    }
    ,                            //4
    {
        QString("1500 km"), 15000.0, 10
    }
    ,                            //5
    {
        QString("1000 km"), 10000.0, 10
    }
    ,                            //6
    {
        QString("700 km"), 7000.0, 11
    }
    ,                            //7
    {
        QString("500 km"), 5000.0, 11
    }
    ,                            //8
    {
        QString("300 km"), 3000.0, 13
    }
    ,                            //9
    {
        QString("200 km"), 2000.0, 13
    }
    ,                            //10
    {
        QString("150 km"), 1500.0, 13
    }
    ,                            //11
    {
        QString("100 km"), 1000.0, 14
    }
    ,                            //12
    {
        QString("70 km"), 700.0, 15
    }
    ,                            //13
    {
        QString("50 km"), 500.0, 16
    }
    ,                            //14
    {
        QString("30 km"), 300.0, 16
    }
    ,                            //15
    {
        QString("20 km"), 200.0, 17
    }
    ,                            //16
    {
        QString("15 km"), 150.0, 17
    }
    ,                            //17
    {
        QString("10 km"), 100.0, 18
    }
    ,                            //18
    {
        QString("7 km"), 70.0, 18
    }
    ,                            //19
    {
        QString("5 km"), 50.0, 19
    }
    ,                            //20
    {
        QString("3 km"), 30.0, 19
    }
    ,                            //21
    {
        QString("2 km"), 20.0, 20
    }
    ,                            //22
    {
        QString("1.5 km"), 15.0, 22
    }
    ,                            //23
    {
        QString("1 km"), 10.0, 24
    }
    ,                            //24
    {
        QString("700 m"), 7.0, 24
    }
    ,                            //25
    {
        QString("500 m"), 5.0, 24
    }
    ,                            //26
    {
        QString("300 m"), 3.0, 24
    }
    ,                            //27
    {
        QString("200 m"), 2.0, 24
    }
    ,                            //28
    {
        QString("150 m"), 1.5, 24
    }
    ,                            //29
    {
        QString("100 m"), 1.0, 24
    }
    ,                            //30
    {
        QString("70 m"), 0.7, 24
    }
    ,                            //31
    {
        QString("50 m"), 0.5, 24
    }
    ,                            //32
    {
        QString("30 m"), 0.3, 24
    }
    ,                            //33
    {
        QString("20 m"), 0.2, 24
    }
    ,                            //34
    {
        QString("15 m"), 0.15, 24
    }
    ,                            //35
    {
        QString("10 m"), 0.10, 24
    }
};

const QString CMapTDB::polyline_typestr[]=
{
    /*0x00,*/   tr(""),
    /*0x01,*/   tr("Major highway"),
    /*0x02,*/   tr("Principal highway"),
    /*0x03,*/   tr("Other highway"),
    /*0x04,*/   tr("Arterial road"),
    /*0x05,*/   tr("Collector road"),
    /*0x06,*/   tr("Residential street"),
    /*0x07,*/   tr("Alley/Private road"),
    /*0x08,*/   tr("Highway ramp, low speed"),
    /*0x09,*/   tr("Highway ramp, high speed"),
    /*0x0a,*/   tr("Unpaved road"),
    /*0x0b,*/   tr("Major highway connector"),
    /*0x0c,*/   tr("Roundabout"),
    /*0x0d,*/   tr(""),
    /*0x0e,*/   tr(""),
    /*0x0f,*/   tr(""),
    /*0x10,*/   tr(""),
    /*0x11,*/   tr(""),
    /*0x12,*/   tr(""),
    /*0x13,*/   tr(""),
    /*0x14,*/   tr("Railroad"),
    /*0x15,*/   tr("Shoreline"),
    /*0x16,*/   tr("Trail"),
    /*0x17,*/   tr(""),
    /*0x18,*/   tr("Stream"),
    /*0x19,*/   tr("Time zone"),
    /*0x1a,*/   tr("Ferry"),
    /*0x1b,*/   tr("Ferry"),
    /*0x1c,*/   tr("State/province border"),
    /*0x1d,*/   tr("County/parish border"),
    /*0x1e,*/   tr("International border"),
    /*0x1f,*/   tr("River"),
    /*0x20,*/   tr("Minor land contour"),
    /*0x21,*/   tr("Intermediate land contour"),
    /*0x22,*/   tr("Major land contour"),
    /*0x23,*/   tr("Minor deph contour"),
    /*0x24,*/   tr("Intermediate depth contour"),
    /*0x25,*/   tr("Major depth contour"),
    /*0x26,*/   tr("Intermittent stream"),
    /*0x27,*/   tr("Airport runway"),
    /*0x28,*/   tr("Pipeline"),
    /*0x29,*/   tr("Powerline"),
    /*0x2a,*/   tr("Marine boundary"),
    /*0x2b,*/   tr("Hazard boundary")
};

CMapTDB::CMapTDB(const QString& key, const QString& filename, CCanvas * parent)
: IMap(eVector, key, parent)
, filename(filename)
, north(-90.0)
, east(-180.0)
, south(90.0)
, west(180.0)
, encrypted(false)
, baseimg(0)
, isTransparent(false)
, zoomFactor(0)
, polylineProperties(0x40)
, polygonProperties(0x80)
, fm(CResources::self().getMapFont())
{
    pjsrc       = pj_init_plus("+proj=merc +ellps=WGS84");

    qDebug() << "pjsrc:\t" << pj_get_def(pjsrc,0);
    qDebug() << "pjtar:\t" << pj_get_def(pjtar,0);

    setup();


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
    resize(parent->size());

    info = new QTextDocument(this);
    info->setTextWidth(TEXTWIDTH);
    info->setHtml(infotext);

    parent->installEventFilter(this);
    qDebug() << "CMapTDB::CMapTDB()";
}

CMapTDB::CMapTDB(const QString& key, const QString& filename)
: IMap(eVector, key, 0)
, filename(filename)
, north(-90.0)
, east(-180.0)
, south(90.0)
, west(180.0)
, encrypted(false)
, baseimg(0)
, isTransparent(false)
, zoomFactor(0)
, polylineProperties(0x40)
, polygonProperties(0x80)
, fm(CResources::self().getMapFont())
{
    pjsrc = pj_init_plus(CMapDB::self().getMap().getProjection());

    qDebug() << getProjection();

    setup();

    readTDB(filename);
    processPrimaryMapData();

    info          = 0;
    isTransparent = true;

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

void CMapTDB::setup()
{
    polylineProperties[0x01] = polyline_property(0x01, "#c46442",   4, Qt::SolidLine);
    polylineProperties[0x02] = polyline_property(0x02, "#dc7c5a",   3, Qt::SolidLine);
    polylineProperties[0x03] = polyline_property(0x03, "#e68664",   2, Qt::SolidLine);
    polylineProperties[0x04] = polyline_property(0x04, "#ffff99",   3, Qt::SolidLine);
    polylineProperties[0x05] = polyline_property(0x05, "#ffff66",   2, Qt::SolidLine);
    polylineProperties[0x06] = polyline_property(0x06, "#FFFFFF",   2, Qt::SolidLine);
    polylineProperties[0x07] = polyline_property(0x07, "#c46442",   1, Qt::SolidLine);
    polylineProperties[0x08] = polyline_property(0x08, "#e88866",   2, Qt::SolidLine);
    polylineProperties[0x09] = polyline_property(0x09, "#e88866",   2, Qt::SolidLine);
    polylineProperties[0x0A] = polyline_property(0x0A, "#808080",   2, Qt::SolidLine);
    polylineProperties[0x0B] = polyline_property(0x0B, "#c46442",   2, Qt::SolidLine);
    polylineProperties[0x0C] = polyline_property(0x0C, "#FFFFFF",   2, Qt::SolidLine);
    polylineProperties[0x14] = polyline_property(0x14, "#FFFFFF",   2, Qt::DotLine);
    polylineProperties[0x15] = polyline_property(0x15, "#000080",   2, Qt::SolidLine);
    polylineProperties[0x16] = polyline_property(0x16, "#E0E0E0",   1, Qt::SolidLine);
    polylineProperties[0x18] = polyline_property(0x18, "#0000ff",   2, Qt::SolidLine);
    polylineProperties[0x19] = polyline_property(0x19, "#00ff00",   2, Qt::SolidLine);
    polylineProperties[0x1A] = polyline_property(0x1A, "#000000",   2, Qt::SolidLine);
    polylineProperties[0x1B] = polyline_property(0x1B, "#000000",   2, Qt::SolidLine);
    polylineProperties[0x1C] = polyline_property(0x1C, "#00c864",   2, Qt::DotLine);
    polylineProperties[0x1D] = polyline_property(0x1D, "#00c864",   2, Qt::DotLine);
    polylineProperties[0x1E] = polyline_property(0x1E, "#00c864",   2, Qt::DotLine);
    polylineProperties[0x1F] = polyline_property(0x1F, "#0000ff",   2, Qt::SolidLine);
    polylineProperties[0x20] = polyline_property(0x20, "#b67824",   1, Qt::SolidLine);
    polylineProperties[0x21] = polyline_property(0x21, "#b67824",   2, Qt::SolidLine);
    polylineProperties[0x22] = polyline_property(0x22, "#b67824",   3, Qt::SolidLine);
    polylineProperties[0x23] = polyline_property(0x23, "#b67824",   1, Qt::SolidLine);
    polylineProperties[0x24] = polyline_property(0x24, "#b67824",   2, Qt::SolidLine);
    polylineProperties[0x25] = polyline_property(0x25, "#b67824",   3, Qt::SolidLine);
    polylineProperties[0x26] = polyline_property(0x26, "#0000ff",   2, Qt::DotLine);
    polylineProperties[0x27] = polyline_property(0x27, "#c46442",   4, Qt::SolidLine);
    polylineProperties[0x28] = polyline_property(0x28, "#aa0000",   2, Qt::SolidLine);
    polylineProperties[0x29] = polyline_property(0x29, "#ff0000",   2, Qt::SolidLine);
    polylineProperties[0x2A] = polyline_property(0x2A, "#000000",   2, Qt::SolidLine);
    polylineProperties[0x2B] = polyline_property(0x2B, "#000000",   2, Qt::SolidLine);

    polygonProperties[0x01] = polygon_property(0x01, Qt::NoPen,     "#d2c0c0", Qt::SolidPattern);
    polygonProperties[0x02] = polygon_property(0x02, Qt::NoPen,     "#fbeab7", Qt::SolidPattern);
    polygonProperties[0x03] = polygon_property(0x03, Qt::NoPen,     "#a4b094", Qt::SolidPattern);
    polygonProperties[0x04] = polygon_property(0x04, Qt::NoPen,     "#808080", Qt::SolidPattern);
    polygonProperties[0x05] = polygon_property(0x05, Qt::NoPen,     "#f0f0f0", Qt::SolidPattern);
    polygonProperties[0x06] = polygon_property(0x06, Qt::NoPen,     "#cacaca", Qt::SolidPattern);
    polygonProperties[0x07] = polygon_property(0x07, Qt::NoPen,     "#feebcf", Qt::SolidPattern);
    polygonProperties[0x08] = polygon_property(0x08, Qt::NoPen,     "#fde8d5", Qt::SolidPattern);
    polygonProperties[0x09] = polygon_property(0x09, Qt::NoPen,     "#fee8b8", Qt::SolidPattern);
    polygonProperties[0x0a] = polygon_property(0x0a, Qt::NoPen,     "#fdeac6", Qt::SolidPattern);
    polygonProperties[0x0b] = polygon_property(0x0b, Qt::NoPen,     "#fddfbd", Qt::SolidPattern);
    polygonProperties[0x0c] = polygon_property(0x0c, Qt::NoPen,     "#ebeada", Qt::SolidPattern);
    polygonProperties[0x0d] = polygon_property(0x0d, Qt::NoPen,     "#f8e3be", Qt::SolidPattern);
    polygonProperties[0x0e] = polygon_property(0x0e, Qt::NoPen,     "#e0e0e0", Qt::SolidPattern);
    polygonProperties[0x13] = polygon_property(0x13, Qt::NoPen,     "#cc9900", Qt::SolidPattern);
    polygonProperties[0x14] = polygon_property(0x14, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x15] = polygon_property(0x15, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x16] = polygon_property(0x16, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x17] = polygon_property(0x17, Qt::NoPen,     "#90be00", Qt::SolidPattern);
    polygonProperties[0x18] = polygon_property(0x18, Qt::NoPen,     "#00ff00", Qt::SolidPattern);
    polygonProperties[0x19] = polygon_property(0x19, Qt::NoPen,     "#f8e3be", Qt::SolidPattern);
    polygonProperties[0x1a] = polygon_property(0x1a, Qt::NoPen,     "#d3f5a5", Qt::SolidPattern);
    polygonProperties[0x1e] = polygon_property(0x1e, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x1f] = polygon_property(0x1f, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x20] = polygon_property(0x20, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x28] = polygon_property(0x28, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x29] = polygon_property(0x29, Qt::NoPen,     "#0000ff", Qt::SolidPattern);
    polygonProperties[0x32] = polygon_property(0x32, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x3b] = polygon_property(0x3b, Qt::NoPen,     "#0000ff", Qt::SolidPattern);
    polygonProperties[0x3c] = polygon_property(0x3c, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x3d] = polygon_property(0x3d, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x3e] = polygon_property(0x3e, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x3f] = polygon_property(0x3f, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x40] = polygon_property(0x40, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x41] = polygon_property(0x41, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x42] = polygon_property(0x42, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x43] = polygon_property(0x43, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x44] = polygon_property(0x44, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x45] = polygon_property(0x45, Qt::NoPen,     "#0000ff", Qt::SolidPattern);
    polygonProperties[0x46] = polygon_property(0x46, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x47] = polygon_property(0x47, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x48] = polygon_property(0x48, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x49] = polygon_property(0x49, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x4a] = polygon_property(0x4a, "#000000",     Qt::NoBrush, Qt::NoBrush);
    polygonProperties[0x4b] = polygon_property(0x4b, "#000000",     Qt::NoBrush, Qt::NoBrush);
    polygonProperties[0x4c] = polygon_property(0x4c, Qt::NoPen,     "#f0e68c", Qt::SolidPattern);
    polygonProperties[0x4d] = polygon_property(0x4d, Qt::NoPen,     "#00ffff", Qt::SolidPattern);
    polygonProperties[0x4e] = polygon_property(0x4e, Qt::NoPen,     "#d3f5a5", Qt::SolidPattern);
    polygonProperties[0x4f] = polygon_property(0x4f, Qt::NoPen,     "#d3f5a5", Qt::SolidPattern);
    polygonProperties[0x50] = polygon_property(0x50, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x51] = polygon_property(0x51, Qt::NoPen,     "#ffffff", Qt::SolidPattern);
    polygonProperties[0x52] = polygon_property(0x52, Qt::NoPen,     "#4aca4a", Qt::SolidPattern);
    polygonProperties[0x53] = polygon_property(0x53, Qt::NoPen,     "#bcedfa", Qt::SolidPattern);
    polygonProperties[0x54] = polygon_property(0x54, Qt::NoPen,     "#fde8d5", Qt::SolidPattern);
    polygonProperties[0x59] = polygon_property(0x59, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x69] = polygon_property(0x69, Qt::NoPen,     "#0080ff", Qt::SolidPattern);

}

void CMapTDB::registerDEM(CMapDEM& dem)
{
    if(pjsrc == 0) {
        dem.deleteLater();
        throw tr("No basemap projection. That shouldn't happen.");
    }

    qDebug() << "Reproject map to:" << dem.getProjection();
    pj_free(pjsrc);
    pjsrc = pj_init_plus(dem.getProjection());
}

void CMapTDB::resize(const QSize& s)
{
    IMap::resize(s);
    topLeftInfo = QPoint(size.width() - TEXTWIDTH - 10 , 10);
}


bool CMapTDB::eventFilter(QObject * watched, QEvent * event)
{

    if(parent() == watched && event->type() == QEvent::MouseMove && !doFastDraw) {
        QMouseEvent * e = (QMouseEvent*)event;

        pointFocus = e->pos();
        QMultiMap<QString, QString> dict;
        getInfoPoints(pointFocus, dict);
        getInfoPois(pointFocus, dict);
        getInfoPolygons(pointFocus, dict);
        getInfoPolylines(pointFocus, dict);

        QString key;
        QStringList keys = dict.uniqueKeys();
        qSort(keys);

        infotext.clear();
        foreach(key,keys) {
            infotext += "<b>" + key + ":</b><br/>";
            const QStringList& values = dict.values(key).toSet().toList();
            infotext += values.join("<br/>");
            infotext += "<br/>";
        }

        if(!doFastDraw) emit sigChanged();
    }

    return IMap::eventFilter(watched, event);
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

    quint32 basemapId   = 0;
    bool    tainted     = false;

    while((quint8*)pRecord < pRawData + data.size()) {
        pRecord->size = gar_load(quint16,pRecord->size);
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

                basemapId       = gar_load(quint32,p->id);
                double n        = GARMIN_RAD((gar_load(qint32,p->north) >> 8) & 0x00FFFFFF);
                double e        = GARMIN_RAD((gar_load(qint32,p->east) >> 8)  & 0x00FFFFFF);
                double s        = GARMIN_RAD((gar_load(qint32,p->south) >> 8) & 0x00FFFFFF);
                double w        = GARMIN_RAD((gar_load(qint32,p->west) >> 8)  & 0x00FFFFFF);

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

                area = QRectF(QPointF(west, north), QPointF(east, south));

            }
            break;

            case 0x4C:           // map tiles
            {
                if(encrypted) break;

                tdb_map_t * p = (tdb_map_t*)pRecord;
                p->id = gar_load(quint32,p->id);
                if(p->id == basemapId) break;

                QString tilename = QString::fromLatin1(p->name);
                // produce a unique key form the tile name and it's ID. Some tiles
                // might have the same name but never the same ID
                //QString key = QString("%1 %2").arg(tilename).arg(p->id,8,10,QChar('0'));
                QString key = QString("%1").arg(p->id,8,10,QChar('0'));

                tile_t& tile    = tiles[key];
                tile.id         = p->id;
                tile.key        = key;
                tile.name       = tilename;
                tile.file.sprintf("%08i.img",p->id);
                tile.file = finfo.dir().filePath(tile.file);

//                 qDebug() << tile.file;

                tile.north  = GARMIN_RAD((gar_load(qint32,p->north) >> 8) & 0x00FFFFFF);
                tile.east   = GARMIN_RAD((gar_load(qint32,p->east) >> 8)  & 0x00FFFFFF);
                tile.south  = GARMIN_RAD((gar_load(qint32,p->south) >> 8) & 0x00FFFFFF);
                tile.west   = GARMIN_RAD((gar_load(qint32,p->west) >> 8)  & 0x00FFFFFF);
                tile.area   = QRectF(QPointF(tile.west, tile.north), QPointF(tile.east, tile.south));

                tile.defAreaU << tile.west << tile.east << tile.east << tile.west;
                tile.defAreaV << tile.north << tile.north << tile.south << tile.south;
                tile.defArea  << QPointF(tile.west, tile.north) << QPointF(tile.east, tile.north) << QPointF(tile.east, tile.south) << QPointF(tile.west, tile.south);

                tile.memSize = 0;
                tdb_map_size_t * s = (tdb_map_size_t*)(p->name + tilename.size() + 1);

                for(quint16 i=0; i < s->count; ++i) {
                    tile.memSize += gar_load(quint32,s->sizes[i]);
                }

                try
                {
                    tile.img = new CGarminTile(this);
                    tile.img->readBasics(tile.file);
                }
                catch(CGarminTile::exce_t e) {

                    if(e.err == CGarminTile::errLock) {
                        tiles.clear();
                        encrypted = true;
                    }
                    else {
                        if(!tainted) {
                            QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Abort,QMessageBox::Abort);
                            tainted = true;
                        }
                        delete tile.img;
                        tile.img = 0;
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
            }
            break;

            case 0x44:
            {
                QString str;
                QTextStream out(&str,QIODevice::WriteOnly);

                out << "<h1>" << name << "</h1>" << endl;

                tdb_copyrights_t * p = (tdb_copyrights_t*)pRecord;
                tdb_copyright_t  * c = &p->entry;
                while((void*)c < (void*)((quint8*)p + gar_load(quint16,p->size) + 3)) {

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
    try
    {
        baseimg = new CGarminTile(this);
        baseimg->readBasics(basemap);
    }
    catch(CGarminTile::exce_t e) {
        // no basemap? bad luck!
        QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Ok,QMessageBox::NoButton);
        deleteLater();
        return false;
    }

    qDebug() << "name:\t\t" << name;
    qDebug() << "basemap:\t" << basemap;
    qDebug() << "dimensions:\t" << "N" << north << "E" << east << "S" << south << "W" << west;

    const QMap<QString,CGarminTile::subfile_desc_t>& subfiles            = baseimg->getSubFiles();
    QMap<QString,CGarminTile::subfile_desc_t>::const_iterator subfile    = subfiles.begin();
    quint8 fewest_map_bits = 0xFF;

    /* Put here so the submap check doesn't do the basemap again. */
    QMap<QString,CGarminTile::subfile_desc_t>::const_iterator basemap_subfile;

    /* Find best candidate for basemap. */
    while (subfile != subfiles.end()) {
        QVector<CGarminTile::maplevel_t>::const_iterator maplevel = subfile->maplevels.begin();
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
    QVector<CGarminTile::maplevel_t>::const_iterator maplevel = basemap_subfile->maplevels.begin();
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

    if(!tiles.isEmpty()) {
        CGarminTile * img = 0;
        QMap<QString,tile_t>::iterator tile = tiles.begin();
        while(tile != tiles.end()) {
            img = tile->img;
            if(img) break;
            ++tile;
        }
        if(img) {
            const QMap<QString,CGarminTile::subfile_desc_t>& subfiles = img->getSubFiles();
            QMap<QString,CGarminTile::subfile_desc_t>::const_iterator subfile = subfiles.begin();
            /*
             * Query all subfiles for possible maplevels.
             * Exclude basemap to avoid polution.
             */
            while (subfile != subfiles.end()) {
                QVector<CGarminTile::maplevel_t>::const_iterator maplevel = subfile->maplevels.begin();
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

            isTransparent = img->isTransparent();
        }
        /* Sort all entries, note that stable sort should insure that basemap is preferred when available. */
        qStableSort(maplevels.begin(), maplevels.end(), map_level_t::GreaterThan);
        /* Delete any duplicates for obvious performance reasons. */
        QVector<map_level_t>::iterator where;
        where = std::unique(maplevels.begin(), maplevels.end());
        maplevels.erase(where, maplevels.end());
    }

#ifdef DEBUG_SHOW_MAPLEVELS
    for(int i=0; i < maplevels.count(); ++i) {
        map_level_t& ml = maplevels[i];
        qDebug() << ml.bits << ml.level << ml.useBaseMap;
    }
#endif

    // read basemap for tile boundary polygons
    // Search all basemap levels for tile boundaries.
    // More detailed polygons will overwrite least detailed ones
    polygons.clear();
    for(int i=0; i < maplevels.count(); ++i) {
        if(!maplevels[i].useBaseMap) break;

        baseimg->loadPolygonsOfType(polygons, 0x4a, maplevels[i].level);

        polytype_t::iterator item = polygons.begin();
        while (item != polygons.end()) {
            if((item->labels.size() > 1) && tiles.contains(item->labels[1])){
                tile_t& tile = tiles[item->labels[1]];

                double * u = item->u.data();
                double * v = item->v.data();
                int N      = item->u.size();

                tile.defArea.clear();

                for(int n = 0; n < N; ++n, ++u, ++v){
                    tile.defArea << QPointF(*u, *v);
                }

                tile.defAreaU = item->u;
                tile.defAreaV = item->v;

            }
            ++item;
        }
    }

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

    for(int i = 0; i < n; ++i, ++u, ++v) {
        *u = (*u - pt.u) / (+1.0 * zoomFactor);
        *v = (*v - pt.v) / (-1.0 * zoomFactor);
    }
};

void CMapTDB::convertRad2Pt(double* u, double* v, int n)
{
    if(pjsrc == 0) {
        return;
    }

    pj_transform(pjtar,pjsrc,n,0,u,v,0);
    convertM2Pt(u,v,n);
}


void CMapTDB::move(const QPoint& old, const QPoint& next)
{
    XY p2 = topLeft;
    IMap::convertRad2Pt(p2.u, p2.v);

    // move top left point by difference
    p2.u += old.x() - next.x();
    p2.v += old.y() - next.y();

    // convert back to new top left geo coordinate
    convertPt2Rad(p2.u, p2.v);
    topLeft = p2;

    needsRedraw = true;
    setFastDraw();
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
    IMap::convertRad2Pt(p1.u, p1.v);

    XY p2 = topLeft;
    IMap::convertRad2Pt(p2.u, p2.v);

    // move top left point by difference point befor and after zoom
    p2.u += p1.u - p0.x();
    p2.v += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2Rad(p2.u, p2.v);
    topLeft = p2;

    //     qDebug() << "maplevel" /*<< mapLevelMap << "(" << mapLevelOvl << ")"*/ << "bits" << scales[zoomidx].bits;
}


void CMapTDB::zoom(double lon1, double lat1, double lon2, double lat2)
{
    double u[3];
    double v[3];
    double dU, dV;

    needsRedraw = true;

    u[0] = lon1;
    v[0] = lat1;
    u[1] = lon2;
    v[1] = lat1;
    u[2] = lon1;
    v[2] = lat2;

    pj_transform(pjtar, pjsrc,3,0,u,v,0);
    dU = u[1] - u[0];
    dV = v[2] - v[0];

    for(int i = MAX_IDX_ZOOM; i >= MIN_IDX_ZOOM; --i) {

        double z    = scales[i].scale;
        double pxU  = dU / (+1.0 * z);
        double pxV  = dV / (-1.0 * z);

        if((pxU < size.width()) && (pxV < size.height())) {
            zoomFactor  = z;
            zoomidx     = i;
            double u = lon1 + (lon2 - lon1)/2;
            double v = lat1 + (lat2 - lat1)/2;
            IMap::convertRad2Pt(u,v);
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

    qDebug() << zoomidx << zoomFactor << scales[zoomidx].bits << scales[zoomidx].scale << scales[zoomidx].label;

    setFastDraw();

    emit sigChanged();
}


void CMapTDB::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = west;
    lat1 = north;
    lon2 = east;
    lat2 = south;
}

void CMapTDB::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{
    p1 = topLeft;
    p2 = bottomRight;

    XY p3;
    p3.u = p1.u;
    p3.v = p2.v;

    my_xscale =  zoomFactor;
    my_yscale = -zoomFactor;

}

void CMapTDB::draw(QPainter& p)
{
    bottomRight.u = size.width();
    bottomRight.v = size.height();
    convertPt2Rad(bottomRight.u, bottomRight.v);

    // render map if necessary
    if(needsRedraw) {
        draw();
    }
    // copy internal buffer to paint device
    p.drawImage(0,0,buffer);

    // render overlay
    if(!ovlMap.isNull() && !doFastDraw){
        ovlMap->draw(size, needsRedraw, p);
    }

    needsRedraw = false;

    if(!infotext.isEmpty() && info) {
        QFont f = p.font();
        f.setBold(false);
        f.setItalic(false);
        info->setDefaultFont(f);
        info->setHtml(infotext);

        p.save();
        p.translate(topLeftInfo);

        QRectF rectInfo(QPointF(0,0), info->size());
        rectInfo.adjust(0,0,4,4);
        rectInfo.moveTopLeft(QPointF(-2,-2));

        p.setPen(Qt::black);
        p.setBrush(QColor(0xff, 0xff, 0xcc, 0xE0));
        p.drawRect(rectInfo);
        info->drawContents(&p);

        p.restore();
    }

    p.drawPixmap(pointFocus - QPoint(5,5), QPixmap(":/icons/small_bullet_yellow.png"));

    if(doFastDraw) setFastDraw();
}

void CMapTDB::draw(const QSize& s, bool needsRedraw, QPainter& p)
{
    int i;

    if(s != size){
        resize(s);
        needsRedraw = true;
    }

    if(needsRedraw){
        float sx, sy;
        getArea_n_Scaling_fromBase(topLeft, bottomRight, sx, sy);

        for(i=0; i < MAX_IDX_ZOOM; ++i){
            if(scales[i].scale <= sx) break;
        }

        zoomidx     = i;
        zoomFactor  = sx;

        draw();

        // make map semi transparent
        quint32 * ptr  = (quint32*)buffer.bits();
        for(int i = 0; i < (buffer.numBytes()>>2); ++i){
            if(*ptr & 0xFF000000){
                *ptr = (*ptr & 0x00FFFFFF) | 0xB0000000;
            }
            ++ptr;
        }
    }

    p.drawImage(0,0,buffer);

    if(ovlMap) ovlMap->draw(s, needsRedraw, p);
}

void CMapTDB::draw()
{
    buffer.fill(Qt::transparent);
    QPainter p(&buffer);

    QFont f = CResources::self().getMapFont();
    fm = QFontMetrics(f);

    p.setFont(f);
    p.setPen(Qt::black);
    p.setBrush(Qt::NoBrush);

    quint8 bits = scales[zoomidx].bits;
    QVector<map_level_t>::const_iterator maplevel = maplevels.end();
    do {
        --maplevel;
        if(bits >= maplevel->bits) break;
    } while(maplevel != maplevels.begin());

    QRectF viewport(QPointF(topLeft.u, topLeft.v), QPointF(bottomRight.u, bottomRight.v));
    polygons.clear();
    polylines.clear();
    pois.clear();
    points.clear();
    labels.clear();

    if(maplevel->useBaseMap) {
        baseimg->loadVisibleData(doFastDraw, polygons, polylines, points, pois, maplevel->level, viewport);
    }
    else {
        QMap<QString,tile_t>::const_iterator tile = tiles.begin();
        while(tile != tiles.end()) {
            if(tile->img && tile->area.intersects(viewport)) {
                tile->img->loadVisibleData(doFastDraw, polygons, polylines, points, pois, maplevel->level, viewport);
            }
            ++tile;
        }
    }

    p.setRenderHint(QPainter::Antialiasing,!doFastDraw);

    if(!doFastDraw && !isTransparent) {
        drawPolygons(p, polygons);
    }
    drawPolylines(p, polylines);
    if(!doFastDraw) {
        drawPoints(p, points);
        drawPois(p, pois);
        drawLabels(p, labels);
    }

}


static quint16 streets[] = {0x16, 0x14, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
// { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x14, 0x16};

static quint16 others[]  = { 0x00, /*0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0A, 0x0B, 0x0C,*/ 0x0D, 0x0E, 0x0F
,0x10, 0x11, 0x12, 0x13, /*0x14,*/ 0x15, /*0x16,*/ 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
,0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
,0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

void CMapTDB::drawPolylines(QPainter& p, polytype_t& lines)
{
    int m;
    const int M = sizeof(others)/sizeof(quint16);

    for(m = 0; m < M; ++m) {
        quint16 type = others[m];

        p.setPen(polylineProperties[type].pen);

        polytype_t::iterator item = lines.begin();
        while(item != lines.end()) {
            if(item->type == type) {
                double * u      = item->u.data();
                double * v      = item->v.data();
                const int size  = item->u.size();

                convertRad2Pt(u,v,size);
                QPolygonF line(size);

                for(int i = 0; i < size; ++i) {
                    line[i].setX(*u++);
                    line[i].setY(*v++);
                }

                p.drawPolyline(line);

                if(!polylineProperties[type].known) qDebug() << "unknown polyline" << hex << type;
            }
            ++item;
        }
    }

    int n;
    const int N = sizeof(streets)/sizeof(quint16);

    for(n = 0; n < N; ++n) {
        quint16 type = streets[n];

        int width = polylineProperties[type].pen.width();
        width = zoomFactor > 7.0 ? width : quint32(width + 7.0/zoomFactor);

        p.setPen(QPen(Qt::black, width + (zoomFactor < 5.0 ? 4 : 2), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        polytype_t::iterator item = lines.begin();
        while(item != lines.end()) {
            if(item->type == type) {
                double * u      = item->u.data();
                double * v      = item->v.data();
                const int size  = item->u.size();

                convertRad2Pt(u,v,size);

                QPolygonF line(size);

                for(int i = 0; i < size; ++i) {
                    line[i].setX(*u++);
                    line[i].setY(*v++);
                }

                p.drawPolyline(line);

            }
            ++item;
        }
    }

    if(doFastDraw) return;

    for(n = 0; n < N; ++n) {
        quint16 type = streets[n];

        QPen pen    = polylineProperties[type].pen;
        int width   = pen.width();
        width       = zoomFactor > 7.0 ? width : quint32(width + 7.0/zoomFactor);
        pen.setWidth(width);

        p.setPen(pen);

        polytype_t::iterator item = lines.begin();
        while(item != lines.end()) {
            if(item->type == type) {
                double * u      = item->u.data();
                double * v      = item->v.data();
                const int size  = item->u.size();

                QPolygonF line(size);

                for(int i = 0; i < size; ++i) {
                    line[i].setX(*u++);
                    line[i].setY(*v++);
                }

                p.drawPolyline(line);
            }
            ++item;
        }
    }
}


static quint16 order[] =
{
      0x4B, 0x53, 0x14, 0x15, 0x16, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
    , 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x17, 0x0F, 0x10, 0x11, 0x12
    , 0x13, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22
    , 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E
    , 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A
    , 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46
    , 0x47, 0x48, 0x49, 0x4A, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x00
    , 0x50, 0x51, 0x52, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C
    , 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68
    , 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74
    , 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x00

//     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
//     , 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
//     , 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
//     , 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
//     , 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F
//     , 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F
//     , 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F
//     , 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F
};

void CMapTDB::drawPolygons(QPainter& p, polytype_t& lines)
{

    int n;
    const int N = sizeof(order)/sizeof(quint16);

    for(n = 0; n < N; ++n) {
        quint16 type = order[n];

        p.setPen(polygonProperties[type].pen);
        p.setBrush(polygonProperties[type].brush);

        polytype_t::iterator item = lines.begin();
        while (item != lines.end()) {
            if(item->type == type) {
                double * u      = item->u.data();
                double * v      = item->v.data();
                const int size  = item->u.size();

                convertRad2Pt(u,v,size);

                QPolygonF line(size);

                for(int i = 0; i < size; ++i) {
                    line[i].setX(*u++);
                    line[i].setY(*v++);
                }

                p.drawPolygon(line);

                if(!polygonProperties[type].known) qDebug() << "unknown polygon" << hex << type;
            }
            ++item;
        }
    }
}


void CMapTDB::drawPoints(QPainter& p, pointtype_t& pts)
{
    if(zoomFactor > 3.0) return;

    pointtype_t::iterator pt = pts.begin();
    while(pt != pts.end()) {
        IMap::convertRad2Pt(pt->lon, pt->lat);
        p.drawPixmap(pt->lon - 4, pt->lat - 4, QPixmap(":/icons/small_bullet_blue.png"));
        if(!pt->labels.isEmpty() && zoomFactor < 2 ) {

            // calculate bounding rectangle with a border of 2 px
            QRect rect = fm.boundingRect(pt->labels.join(" "));
            rect.adjust(0,0,4,4);
            rect.moveCenter(QPoint(pt->lon, pt->lat));

            // test rectangle for intersection with existng labels
            QVector<strlbl_t>::const_iterator label = labels.begin();
            while(label != labels.end()) {
                if(label->rect.intersects(rect)) break;
                ++label;
            }

            // if no intersection was found, add label to list
            if(label == labels.end()) {
                labels.push_back(strlbl_t());
                strlbl_t& strlbl = labels.last();
                strlbl.pt   = QPoint(pt->lon, pt->lat);
                strlbl.str  = pt->labels.join(" ");
                strlbl.rect = rect;
            }
        }
        ++pt;
    }
}


void CMapTDB::drawPois(QPainter& p, pointtype_t& pts)
{

    pointtype_t::iterator pt = pts.begin();
    while(pt != pts.end()) {
        IMap::convertRad2Pt(pt->lon, pt->lat);
        p.drawPixmap(pt->lon - 4, pt->lat - 4, QPixmap(":/icons/small_bullet_red.png"));
        if(!pt->labels.isEmpty()) {

            // calculate bounding rectangle with a border of 2 px
            QRect rect = fm.boundingRect(pt->labels.join(" "));
            rect.adjust(0,0,4,4);
            rect.moveCenter(QPoint(pt->lon, pt->lat));

            // test rectangle for intersection with existng labels
            QVector<strlbl_t>::const_iterator label = labels.begin();
            while(label != labels.end()) {
                if(label->rect.intersects(rect)) break;
                ++label;
            }

            // if no intersection was found, add label to list
            if(label == labels.end()) {
                labels.push_back(strlbl_t());
                strlbl_t& strlbl = labels.last();
                strlbl.pt   = QPoint(pt->lon, pt->lat);
                strlbl.str  = pt->labels.join(" ");
                strlbl.rect = rect;
            }
        }
        ++pt;
    }
}


void CMapTDB::drawLabels(QPainter& p, QVector<strlbl_t> lbls)
{
    QVector<strlbl_t>::const_iterator lbl = lbls.begin();
    while(lbl != lbls.end()) {
        CCanvas::drawText(lbl->str, p, lbl->pt, Qt::black);
        ++lbl;
    }
}




void CMapTDB::getInfoPoints(const QPoint& pt, QMultiMap<QString, QString>& dict)
{
    pointtype_t::const_iterator point = points.begin();
    while(point != points.end()) {
        QPoint x = pt - QPoint(point->lon, point->lat);
        if(x.manhattanLength() < 10) {
            dict.insert(tr("Point of Interest"),point->labels.join(", "));
        }
        ++point;
    }
}


void CMapTDB::getInfoPois(const QPoint& pt, QMultiMap<QString, QString>& dict)
{
    pointtype_t::const_iterator point = pois.begin();
    while(point != pois.end()) {
        QPoint x = pt - QPoint(point->lon, point->lat);
        if(x.manhattanLength() < 10) {
            dict.insert(tr("Point of Interest"),point->labels.join(", "));
        }
        ++point;
    }
}


void CMapTDB::getInfoPolylines(QPoint& pt, QMultiMap<QString, QString>& dict)
{
    int i = 0;                   // index into poly line
    int len;                     // number of points in line
    XY p1, p2;                   // the two points of the polyline close to pt
    double dx,dy;                // delta x and y defined by p1 and p2
    double d_p1_p2;              // distance between p1 and p2
    double u;                    // ratio u the tangent point will divide d_p1_p2
    double x,y;                  // coord. (x,y) of the point on line defined by [p1,p2] close to pt
    double distance;             // the distance to the polyline
    double shortest;             // shortest distance sofar

    QPointF resPt = pt;
    QString key, value;

    shortest = 50;

    polytype_t::const_iterator line = polylines.begin();
    while(line != polylines.end()) {
        len = line->u.count();
        // need at least 2 points
        if(len < 2) {
            ++line;
            continue;
        }

        // see http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/
        for(i=1; i<len; ++i) {
            p1.u = line->u[i-1];
            p1.v = line->v[i-1];
            p2.u = line->u[i];
            p2.v = line->v[i];

            dx = p2.u - p1.u;
            dy = p2.v - p1.v;

            d_p1_p2 = sqrt(dx * dx + dy * dy);

            u = ((pt.x() - p1.u) * dx + (pt.y() - p1.v) * dy) / (d_p1_p2 * d_p1_p2);

            if(u < 0.0 || u > 1.0) continue;

            x = p1.u + u * dx;
            y = p1.v + u * dy;

            distance = sqrt((x - pt.x())*(x - pt.x()) + (y - pt.y())*(y - pt.y()));

            if(distance < shortest) {

                key = polyline_typestr[line->type];

                if(!line->labels.isEmpty()) {
                    switch(line->type) {
                                 // "Minor depht contour"
                        case 0x23:
                                 // "Minor land contour"
                        case 0x20:
                                 // "Intermediate depth contour",
                        case 0x24:
                                 // "Intermediate land contour",
                        case 0x21:
                                 // "Major depth contour",
                        case 0x25:
                                 // "Major land contour",
                        case 0x22:
                        {
                            QString unit;
                            QString val = line->labels[0];
                            IUnit::self().meter2elevation(val.toFloat() / 3.28084f, val, unit);
                            value = QString("%1 %2").arg(val).arg(unit);
                        }
                        break;

                        default:
                            value = line->labels.join(" ").simplified();
                    }
                }
                else {
                    value = "-";
                }
                resPt.setX(x);
                resPt.setY(y);
                shortest = distance;
            }

        }
        ++line;
    }

    if(!key.isEmpty()) {
        dict.insert(key,value);
    }

    pt = resPt.toPoint();
}


void CMapTDB::getInfoPolygons(const QPoint& pt, QMultiMap<QString, QString>& dict)
{
    int     npol;
    int     i = 0, j = 0 ,c = 0;
    XY      p1, p2;              // the two points of the polyline close to pt
    double  x = pt.x();
    double  y = pt.y();
    QString value;

    polytype_t::const_iterator line = polygons.begin();
    while(line != polygons.end()) {

        npol = line->u.count();
        if(npol > 2) {
            c = 0;
            // see http://local.wasp.uwa.edu.au/~pbourke/geometry/insidepoly/
            for (i = 0, j = npol-1; i < npol; j = i++) {
                p1.u = line->u[j];
                p1.v = line->v[j];
                p2.u = line->u[i];
                p2.v = line->v[i];

                if ((((p2.v <= y) && (y < p1.v))  || ((p1.v <= y) && (y < p2.v))) &&
                (x < (p1.u - p2.u) * (y - p2.v) / (p1.v - p2.v) + p2.u)) {
                    c = !c;
                }
            }

            if(c && !line->labels.isEmpty()) {
                dict.insert(tr("Area"), line->labels.join(" ").simplified());
            }
        }
        ++line;
    }
}


void CMapTDB::select(IMapSelection& ms, const QRect& rect)
{
    CMapSelectionGarmin& sel = (CMapSelectionGarmin&)ms;

    double lon1 = rect.left();
    double lat1 = rect.top();
    convertPt2Rad(lon1, lat1);

    double lon2 = rect.right();
    double lat2 = rect.bottom();
    convertPt2Rad(lon2, lat2);

    QPolygonF poly;
    poly << QPointF(lon1, lat1) << QPointF(lon2, lat1) << QPointF(lon2, lat2) << QPointF(lon1, lat2);


    if(!sel.maps.contains(key)){
        sel.maps[key];
    }
    QMap<QString, CMapSelectionGarmin::map_t>::iterator map = sel.maps.find(key);

    QMap<QString,tile_t>::iterator tile = tiles.begin();
    while(tile != tiles.end()){
        QPolygonF res = poly.intersected(tile->defArea);

        if(!res.isEmpty()){

            if(map->tiles.contains(tile->key)){
                qDebug() << "-" << tile->key;

                map->tiles.remove(tile->key);
            }
            else{
                qDebug() << "+" << tile->key;

                CMapSelectionGarmin::tile_t t;
                t.filename  = tile->file;
                t.u         = tile->defAreaU;
                t.v         = tile->defAreaV;
                t.memSize   = tile->memSize;
                map->tiles[tile->key] = t;
            }
        }

        ++tile;
    }

    sel.maps[key] = *map;


    quint32 memSize = 0;
    sel.description += QString("\nSize: %1 MB").arg(memSize / (1024 * 1024));
}
