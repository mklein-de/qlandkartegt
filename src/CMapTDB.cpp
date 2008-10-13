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

CMapTDB::CMapTDB(const QString& key, const QString& filename, CCanvas * parent)
: IMap(key,parent)
{

}

CMapTDB::~CMapTDB()
{

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
}

