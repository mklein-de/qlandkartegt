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

#define MAX_IDX_ZOOM 35
#define MIN_IDX_ZOOM 0

CMapJnx::scale_t CMapJnx::scales[] =
{
    {                            //0
        QString("7000 km"), 70000.0
    }
    ,                            //1
    {
        QString("5000 km"), 50000.0
    }
    ,                            //2
    {
        QString("3000 km"), 30000.0
    }
    ,                            //3
    {
        QString("2000 km"), 20000.0
    }
    ,                            //4
    {
        QString("1500 km"), 15000.0
    }
    ,                            //5
    {
        QString("1000 km"), 10000.0
    }
    ,                            //6
    {
        QString("700 km"), 7000.0
    }
    ,                            //7
    {
        QString("500 km"), 5000.0
    }
    ,                            //8
    {
        QString("300 km"), 3000.0
    }
    ,                            //9
    {
        QString("200 km"), 2000.0
    }
    ,                            //10
    {
        QString("150 km"), 1500.0
    }
    ,                            //11
    {
        QString("100 km"), 1000.0
    }
    ,                            //12
    {
        QString("70 km"), 700.0
    }
    ,                            //13
    {
        QString("50 km"), 500.0
    }
    ,                            //14
    {
        QString("30 km"), 300.0
    }
    ,                            //15
    {
        QString("20 km"), 200.0
    }
    ,                            //16
    {
        QString("15 km"), 150.0
    }
    ,                            //17
    {
        QString("10 km"), 100.0
    }
    ,                            //18
    {
        QString("7 km"), 70.0
    }
    ,                            //19
    {
        QString("5 km"), 50.0
    }
    ,                            //20
    {
        QString("3 km"), 30.0
    }
    ,                            //21
    {
        QString("2 km"), 20.0
    }
    ,                            //22
    {
        QString("1.5 km"), 15.0
    }
    ,                            //23
    {
        QString("1 km"), 10.0
    }
    ,                            //24
    {
        QString("700 m"), 7.0
    }
    ,                            //25
    {
        QString("500 m"), 5.0
    }
    ,                            //26
    {
        QString("300 m"), 3.0
    }
    ,                            //27
    {
        QString("200 m"), 2.0
    }
    ,                            //28
    {
        QString("150 m"), 1.5
    }
    ,                            //29
    {
        QString("100 m"), 1.0
    }
    ,                            //30
    {
        QString("70 m"), 0.7
    }
    ,                            //31
    {
        QString("50 m"), 0.5
    }
    ,                            //32
    {
        QString("30 m"), 0.3
    }
    ,                            //33
    {
        QString("20 m"), 0.2
    }
    ,                            //34
    {
        QString("15 m"), 0.15
    }
    ,                            //35
    {
        QString("10 m"), 0.10
    }
};


CMapJnx::CMapJnx(const QString& key, const QString& fn, CCanvas * parent)
: IMap(eRaster,key,parent)
, xscale(1.0)
, yscale(-1.0)
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
            double tLat1, tLon1, tLat2, tLon2;
            qint32 iLat1, iLon1, iLat2, iLon2;
            tile_t& tile = level.tiles[m];

            stream >> iLat2 >> iLon2 >> iLat1 >> iLon1 ;
            stream >> tile.width >> tile.height >> tile.size >> tile.offset;

            tLat1 = iLat1 * 180.0 / 0x7FFFFFFF;
            tLon1 = iLon1 * 180.0 / 0x7FFFFFFF;
            tLat2 = iLat2 * 180.0 / 0x7FFFFFFF;
            tLon2 = iLon2 * 180.0 / 0x7FFFFFFF;

            tile.area.setLeft(tLon1);
            tile.area.setRight(tLon2);
            tile.area.setTop(tLat2);
            tile.area.setBottom(tLat1);
        }
    }



    pjsrc   = pj_init_plus("+proj=merc +ellps=WGS84 +datum=WGS84 +no_defs");

    x = lon1 * DEG_TO_RAD;
    y = lat2 * DEG_TO_RAD;

    pj_transform(pjtar, pjsrc,1,0,&x,&y,0);

    QSettings cfg;
    cfg.beginGroup("birdseye/maps");
    cfg.beginGroup(name);
    zoomidx = cfg.value("zoomidx",23).toInt();
    x       = cfg.value("x",x).toDouble();
    y       = cfg.value("y",y).toDouble();
    cfg.endGroup();
    cfg.endGroup();

    zoom(zoomidx);
}

CMapJnx::~CMapJnx()
{
    if(pjsrc) pj_free(pjsrc);

    QSettings cfg;
    cfg.beginGroup("birdseye/maps");
    cfg.beginGroup(name);
    cfg.setValue("zoomidx",zoomidx);
    cfg.setValue("x",x);
    cfg.setValue("y",y);
    cfg.endGroup();
    cfg.endGroup();

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
    XY p2;
    p2.u = x;
    p2.v = y;
    convertM2Pt(p2.u, p2.v);

    // move top left point by difference
    p2.u += old.x() - next.x();
    p2.v += old.y() - next.y();

    // convert back to new top left geo coordinate
    convertPt2M(p2.u, p2.v);
    x = p2.u;
    y = p2.v;

    needsRedraw = true;
    emit sigChanged();

}

void CMapJnx::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

    needsRedraw = true;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    zoomidx += zoomIn ? +1 : -1;
    // sigChanged will be sent at the end of this function
    blockSignals(true);
    zoom(zoomidx);

    // convert geo. coordinates back to point
    IMap::convertRad2Pt(p1.u, p1.v);

    XY p2;
    p2.u = x;
    p2.v = y;
    convertM2Pt(p2.u, p2.v);

    // move top left point by difference point befor and after zoom
    p2.u += p1.u - p0.x();
    p2.v += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2M(p2.u, p2.v);
    x = p2.u;
    y = p2.v;

    blockSignals(false);
    emit sigChanged();

}

void CMapJnx::zoom(double lon1, double lat1, double lon2, double lat2)
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

    for(int i = MAX_IDX_ZOOM; i >= MIN_IDX_ZOOM; --i)
    {

        double z    = scales[i].scale;
        double pxU  = dU / (+1.0 * z);
        double pxV  = dV / (-1.0 * z);

        if(isLonLat())
        {
            pxU /= xscale;
            pxV /= yscale;
        }

        if((fabs(pxU) < size.width()) && (fabs(pxV) < size.height()))
        {
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

void CMapJnx::zoom(qint32& level)
{
    needsRedraw = true;

    zoomidx = level;
    if(zoomidx < MIN_IDX_ZOOM) zoomidx = MIN_IDX_ZOOM;
    if(zoomidx > MAX_IDX_ZOOM) zoomidx = MAX_IDX_ZOOM;
    zoomFactor = scales[zoomidx].scale;

    qDebug() << zoomidx << zoomFactor << scales[zoomidx].scale << scales[zoomidx].label;

    emit sigChanged();
}

void CMapJnx::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = this->lon1 * DEG_TO_RAD;
    lon2 = this->lon2 * DEG_TO_RAD;
    lat1 = this->lat2 * DEG_TO_RAD;
    lat2 = this->lat1 * DEG_TO_RAD;

}

void CMapJnx::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    if(needsRedraw)
    {
        draw();
    }


    p.drawImage(0,0,buffer);

    if(!ovlMap.isNull() && !doFastDraw)
    {
        ovlMap->draw(size, needsRedraw, p);
    }

    needsRedraw = false;
}

void CMapJnx::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{

    p1.u = x;
    p1.v = y;
    convertM2Rad(p1.u, p1.v);

    p2.u = x;
    p2.v = y;
    convertM2Pt(p2.u, p2.v);

    p2.u += size.width();
    p2.v += size.height();

    convertPt2Rad(p2.u, p2.v);


    my_xscale   = xscale*zoomFactor;
    my_yscale   = yscale*zoomFactor;
}


qint32 CMapJnx::zlevel2idx(quint32 l)
{
    quint32 index   = -1;
    double d        = 50;

    const quint32 N = levels.size();
    for(quint32 i=0; i < N; i++)
    {
        level_t& level = levels[i];
        double s1 = double(level.scale) * 3 / (2*PI*100);
        double s2 = scales[l].scale;

        if((fabs(s1-s2) < d) && (fabs(s1-s2) < 40))
        {
            index = i;
            d = fabs(s1-s2);
        }

    }
    return index;
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


    qint32 level = zlevel2idx(zoomidx);
    if(level < 0)
    {
        double u1 = lon1 * DEG_TO_RAD;
        double u2 = lon2 * DEG_TO_RAD;
        double v2 = lat2 * DEG_TO_RAD;
        double v1 = lat1 * DEG_TO_RAD;

        convertRad2Pt(u1,v1);
        convertRad2Pt(u2,v2);

        QRectF r;
        r.setLeft(u1);
        r.setRight(u2);
        r.setTop(v2);
        r.setBottom(v1);

        p.setPen(QPen(Qt::darkBlue,2));
        p.setBrush(QBrush(QColor(230,230,255,100) ));
        p.drawRect(r);
        return;
    }


    QByteArray data(1024*1024*4,0);
    data[0] = 0xFF;
    data[1] = 0xD8;
    char * pData = data.data() + 2;

    QImage image;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QVector<tile_t>& tiles = levels[level].tiles;
    const quint32 M = tiles.size();
    for(quint32 m = 0; m < M; m++)
    {
        tile_t& tile = tiles[m];

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
            file.read(pData, tile.size);
            image.loadFromData(data);

            p.drawImage(r, image.scaled(r.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }
}


