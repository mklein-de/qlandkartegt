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

CMapJnx::CMapJnx(const QString& key, const QString& filename, CCanvas * parent)
: IMap(eRaster,key,parent)
{

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
