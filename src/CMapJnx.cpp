/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#include "CMapJnx.h"
#include "CResources.h"
#include <QtGui>

CMapJnx::CMapJnx(const QString& key, const QString& fn, CCanvas * parent)
: IMap(eRaster,key,parent)
, xscale(1.0)
, yscale(-1.0)
, zoomFactor(10.0)
{
    hdr_t hdr;

    filename = fn;

    QFileInfo fi(fn);
    name = fi.fileName();
    name = name.left(name.size() - 4);

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> hdr.version;            // byte 00000000..00000003
    stream >> hdr.devid;              // byte 00000004..00000007
    stream >> hdr.iLat2;              // byte 00000008..0000000B
    stream >> hdr.iLon2;              // byte 0000000C..0000000F
    stream >> hdr.iLat1;              // byte 00000010..00000013
    stream >> hdr.iLon1;              // byte 00000014..00000017
    stream >> hdr.details;            // byte 00000018..0000001B
    stream >> hdr.expire;             // byte 0000001C..00000023
    stream >> hdr.crc;                // byte 00000024..00000027
    stream >> hdr.signature;          // byte 00000028..0000002B
    stream >> hdr.signature_offset;   // byte 0000002C..0000002F

    lat1 = hdr.iLat1 * 180.0 / 0x7FFFFFFF;
    lat2 = hdr.iLat2 * 180.0 / 0x7FFFFFFF;
    lon1 = hdr.iLon1 * 180.0 / 0x7FFFFFFF;
    lon2 = hdr.iLon2 * 180.0 / 0x7FFFFFFF;

    qDebug() << filename;
    qDebug() << hex << "Version:" << hdr.version << "DevId" <<  hdr.devid;
    qDebug() << lon1 << lat1 << lon2 << lat2;
    qDebug() << hex <<  hdr.iLat1 <<  hdr.iLon1 <<  hdr.iLat2 <<  hdr.iLon2;
    qDebug() << hex << "Details:" <<  hdr.details << "Expire:" <<  hdr.expire << "CRC:" <<  hdr.crc ;
    qDebug() << hex << "Signature:" <<  hdr.signature << "Offset:" <<  hdr.signature_offset;

    qDebug() << "Levels:";
    levels.resize(hdr.details);
    for(quint32 i = 0; i < hdr.details; i++)
    {
        level_t& level = levels[i];
        stream >> level.nTiles >> level.offset >> level.scale;

        qDebug() << i << hex << level.nTiles << level.offset << level.scale;
    }

    for(quint32 i = 0; i < hdr.details; i++)
    {

        level_t& level = levels[i];
        const quint32 M = level.nTiles;
        file.seek(level.offset);

        level.tiles.resize(M);

        for(quint32 m = 0; m < M; m++)
        {
            double lat1, lon1, lat2, lon2;
            qint32 iLat1, iLon1, iLat2, iLon2;
            tile_t& tile = level.tiles[m];

            stream >> iLat2 >> iLon2 >> iLat1 >> iLon1 ;
            stream >> tile.width >> tile.height >> tile.size >> tile.offset;

            lat1 = iLat1 * 180.0 / 0x7FFFFFFF;
            lon1 = iLon1 * 180.0 / 0x7FFFFFFF;
            lat2 = iLat2 * 180.0 / 0x7FFFFFFF;
            lon2 = iLon2 * 180.0 / 0x7FFFFFFF;

            tile.area.setLeft(lon1);
            tile.area.setRight(lon2);
            tile.area.setTop(lat2);
            tile.area.setBottom(lat1);

//            tile.area = tile.area.normalized();

//            qDebug() << m << tile.area << lat1 << lon1 << lat2 << lon2;
//            qDebug() << "    " << tile.width << tile.height << tile.size << hex << tile.offset;
        }
    }

    pjsrc   = pj_init_plus("+proj=merc +ellps=WGS84 +datum=WGS84 +no_defs");

    x = lon1 * DEG_TO_RAD;
    y = lat2 * DEG_TO_RAD;

    pj_transform(pjtar, pjsrc,1,0,&x,&y,0);

    zoomidx = 1;
}

CMapJnx::~CMapJnx()
{
    if(pjsrc) pj_free(pjsrc);

}

void CMapJnx::convertPt2M(double& u, double& v)
{
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;
}

void CMapJnx::convertM2Pt(double& u, double& v)
{
    u = (u - x) / (xscale * zoomFactor);
    v = (v - y) / (yscale * zoomFactor);
}

void CMapJnx::move(const QPoint& old, const QPoint& next)
{

}

void CMapJnx::zoom(bool zoomIn, const QPoint& p)
{

}

void CMapJnx::zoom(double lon1, double lat1, double lon2, double lat2)
{

}
void CMapJnx::zoom(qint32& level)
{

}

void CMapJnx::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{

}

void CMapJnx::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    draw();


    p.drawImage(0,0,buffer);

    // render overlay
    if(!ovlMap.isNull() && !doFastDraw)
    {
        qDebug() << size << needsRedraw;
        ovlMap->draw(size, needsRedraw, p);
    }

    if(CResources::self().showZoomLevel())
    {

        QString str;
        if(zoomFactor < 1.0)
        {
            str = tr("Overzoom x%1").arg(1/zoomFactor,0,'f',0);
        }
        else
        {
            str = tr("Zoom level x%1").arg(zoomidx);
        }


        p.setPen(Qt::white);
        p.setFont(QFont("Sans Serif",14,QFont::Black));

        p.drawText(9  ,23, str);
        p.drawText(10 ,23, str);
        p.drawText(11 ,23, str);
        p.drawText(9  ,24, str);
        p.drawText(11 ,24, str);
        p.drawText(9  ,25, str);
        p.drawText(10 ,25, str);
        p.drawText(11 ,25, str);

        p.setPen(Qt::darkBlue);
        p.drawText(10,24,str);
    }
}

void CMapJnx::draw()
{
    if(pjsrc == 0) return IMap::draw();

    buffer.fill(Qt::white);
    QPainter p(&buffer);

    p.setBrush(Qt::NoBrush);


    double u1 = 0;
    double v1 = size.height();
    double u2 = size.width();
    double v2 = 0;

    convertPt2Rad(u1,v1);
    convertPt2Rad(u2,v2);

    u1 *= RAD_TO_DEG;
    v1 *= RAD_TO_DEG;
    u2 *= RAD_TO_DEG;
    v2 *= RAD_TO_DEG;

    viewport.setLeft(u1);
    viewport.setRight(u2);
    viewport.setTop(v2);
    viewport.setBottom(v1);

//    viewport = viewport.normalized();

    qDebug() << viewport << u1 << v1 << u2 << v2;


    QByteArray SOI(2,0);
    SOI[0] = 0xFF;
    SOI[1] = 0xD8;

    int cnt = 0;
    QImage image;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QVector<tile_t>& tiles = levels[2].tiles;
    const quint32 M = tiles.size();
    for(quint32 m = 0; m < M; m++)
    {
        tile_t& tile = tiles[m];

//        qDebug() << viewport << tile.area;

        if(viewport.intersects(tile.area))
        {
            double u1 = tile.area.left() * DEG_TO_RAD;
            double u2 = tile.area.right() * DEG_TO_RAD;
            double v2 = tile.area.top() * DEG_TO_RAD;
            double v1 = tile.area.bottom() * DEG_TO_RAD;

            convertRad2Pt(u1,v1);
            convertRad2Pt(u2,v2);

            QRectF r;
            r.setLeft(u1);
            r.setRight(u2);
            r.setTop(v2);
            r.setBottom(v1);

            file.seek(tile.offset);
            QByteArray data =  SOI + file.read(tile.size);

            image.loadFromData(data);

            p.drawImage(r, image.scaled(r.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        }
    }
//    QByteArray SOI(2,0);
//    SOI[0] = 0xFF;
//    SOI[1] = 0xD8;

//    int cnt = 0;
//    QImage image;
//    QFile file(filename);
//    file.open(QIODevice::ReadOnly);

//    QVector<tile_t>& tiles = levels[0].tiles;
//    const quint32 M = tiles.size();
//    for(quint32 m = 0; m < M; m++)
//    {
//        tile_t& tile = tiles[m];
//        if(viewport.intersects(tile.area))
//        {
////            qDebug() << m << tile.area << viewport;
//            double u1 = tile.area.left() * DEG_TO_RAD;
//            double v1 = tile.area.top() * DEG_TO_RAD;
//            double u2 = tile.area.right() * DEG_TO_RAD;
//            double v2 = tile.area.bottom() * DEG_TO_RAD;

//            convertRad2Pt(u1,v1);
//            convertRad2Pt(u2,v2);

//            v1 = size.height() + v1;


////            qDebug() << u1 << v1 << u2 << v2;

//            QRectF r = QRectF(u1,v1,  u1 - u2, v1 - v2);

////            qDebug() << r;

////            qDebug() << m << tile.area << viewport << r;

//            file.seek(tile.offset);
//            QByteArray data =  SOI + file.read(tile.size);

//            image.loadFromData(data);

//            p.drawImage(r, image.scaled(r.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

//            cnt++;
////            if(cnt == 20) break;
//        }
//    }
}


