/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
#include "CGarminTypNT.h"

CGarminTypNT::CGarminTypNT(QObject * parent)
: IGarminTyp(eNT, parent)
{

}

CGarminTypNT::~CGarminTypNT()
{

}

bool CGarminTypNT::decode(QDataStream& in, QMap<quint32, polygon_property>& polygons, QMap<quint32, polyline_property>& polylines, QList<quint32>& drawOrder, QMap<quint32, QImage>& points)
{
    quint8 tmp8;

    if(!parseHeader(in))
    {
        return false;
    }

    in >> sectNT.arrayOffset >> sectNT.arrayModulo >> sectNT.arraySize >> tmp8 >> sectNT.dataOffset >> sectNT.dataLength;

    qDebug() << "NT         doff/dlen/aoff/amod/asize:" << hex << "\t" << sectNT.dataOffset << "\t" << sectNT.dataLength << "\t" << sectNT.arrayOffset << "\t" << sectNT.arrayModulo << "\t" << sectNT.arrayOffset;

    if(!parsePolygon(in, polygons))
    {
        return false;
    }

    if(!parseDrawOrder(in, drawOrder))
    {
        return false;
    }



    return true;
}
