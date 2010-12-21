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
#include <QtGui>

CMapJnx::CMapJnx(const QString& key, const QString& fn, CCanvas * parent)
: IMap(eRaster,key,parent)
{
    hdr_t hdr;

    filename = fn;
    name     = fn;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> hdr.version;            // byte 00000000..00000003
    stream >> hdr.devid;              // byte 00000004..00000007
    stream >> hdr.iLat1;              // byte 00000008..0000000B
    stream >> hdr.iLon1;              // byte 0000000C..0000000F
    stream >> hdr.iLat2;              // byte 00000010..00000013
    stream >> hdr.iLon2;              // byte 00000014..00000017
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
            qint32 lat1, lon1, lat2, lon2;
            tile_t& tile = level.tiles[m];

            stream >> lat1 >> lon1 >> lat2 >> lon2 ;
            stream >> tile.width >> tile.height >> tile.size >> tile.offset;

            tile.lon1 = lon1 * 180.0 / 0x7FFFFFFF;
            tile.lat1 = lat1 * 180.0 / 0x7FFFFFFF;
            tile.lon2 = lon2 * 180.0 / 0x7FFFFFFF;
            tile.lat2 = lat2 * 180.0 / 0x7FFFFFFF;

            qDebug() << m << tile.lon1 << tile.lat1 << tile.lon2 << tile.lat2;
            qDebug() << "    " << tile.width << tile.height << tile.size << hex << tile.offset;
        }
    }
}

CMapJnx::~CMapJnx()
{

}

void CMapJnx::convertPt2M(double& u, double& v)
{

}

void CMapJnx::convertM2Pt(double& u, double& v)
{

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

