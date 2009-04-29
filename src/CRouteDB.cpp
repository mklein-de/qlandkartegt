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

#include "CRouteDB.h"

CRouteDB * CRouteDB::m_self = 0;

CRouteDB::CRouteDB(QTabWidget * tb, QObject * parent)
: IDB(tb, parent)
{
    m_self = this;
}

CRouteDB::~CRouteDB()
{

}

/// load database data from gpx
void CRouteDB::loadGPX(CGpx& gpx)
{

}

/// save database data to gpx
void CRouteDB::saveGPX(CGpx& gpx)
{

}

/// load database data from QLandkarte binary
void CRouteDB::loadQLB(CQlb& qlb)
{

}

/// save database data to QLandkarte binary
void CRouteDB::saveQLB(CQlb& qlb)
{

}

void CRouteDB::upload()
{

}

void CRouteDB::download()
{

}

void CRouteDB::clear()
{

}

