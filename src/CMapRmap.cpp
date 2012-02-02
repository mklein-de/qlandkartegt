/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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
#include "CMapRmap.h"


#include <QtGui>

CMapRmap::CMapRmap(const QString &key, const QString &fn, CCanvas *parent)
: IMap(eRaster,key,parent)
{
    filename = fn;

    QFileInfo fi(fn);
    name = fi.fileName();
    name = name.left(name.size() - 5);

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    QByteArray magic(20,0);
    stream.readRawData(magic.data(), 19);

    if("CompeGPSRasterImage" != QString(magic))
    {
        qDebug() << QString(magic);
        QMessageBox::warning(0, tr("Error..."), tr("This is not a TwoNav RMAP file."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    quint32 tmp32;
    stream >> tmp32 >> tmp32 >> tmp32;

    qint32 width;
    qint32 height;

    stream >> width >> height;
    qDebug() << width << height << hex << width << height;

    stream >> tmp32 >> tmp32;

    qint32 tileWidth;
    qint32 tileHeight;

    stream >> tileWidth >> tileHeight;
    qDebug() << tileWidth << tileHeight << hex << tileWidth << tileHeight;

    quint64 mapDataOffset;
    stream >> mapDataOffset;
    qDebug() << mapDataOffset << hex << (quint32)mapDataOffset;

    stream >> tmp32;
    quint32 nZoomLevels;
    stream >> nZoomLevels;
    qDebug() << nZoomLevels << hex << nZoomLevels;




}

CMapRmap::~CMapRmap()
{

}

void CMapRmap::convertPt2M(double& u, double& v)
{

}

void CMapRmap::convertM2Pt(double& u, double& v)
{

}


void CMapRmap::move(const QPoint& old, const QPoint& next)
{

}

void CMapRmap::zoom(bool zoomIn, const QPoint& p)
{

}

void CMapRmap::zoom(double lon1, double lat1, double lon2, double lat2)
{

}

void CMapRmap::zoom(qint32& level)
{

}

void CMapRmap::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{

}

void CMapRmap::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{

}
