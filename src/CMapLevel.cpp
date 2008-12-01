/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CMapLevel.h"
#include "CMapFile.h"
#include "CMapDEM.h"
#include "CMapQMAP.h"
#include "CWpt.h"

CMapLevel::CMapLevel(quint32 min, quint32 max, CMapQMAP * parent)
: QObject(parent)
, min(min)
, max(max)
, pjtar(0)
, pjsrc(0)
, westbound(180)
, northbound(-90)
, eastbound(-180)
, southbound(90)
{
    pjtar = pj_init_plus("+proj=longlat  +datum=WGS84 +no_defs");
}


CMapLevel::~CMapLevel()
{
    if(pjtar) pj_free(pjtar);
    if(pjsrc) pj_free(pjsrc);
}


void CMapLevel::addMapFile(const QString& filename)
{
    CMapFile * mapfile = new CMapFile(filename,this);
    if(mapfile && !mapfile->ok) {
        delete mapfile;
        return;
    }
    mapfiles << mapfile;
    Q_ASSERT((*mapfiles.begin())->strProj == mapfile->strProj);
    if(pjsrc == 0) {
        pjsrc = pj_init_plus(mapfile->strProj.toLatin1());
    }

    double n = 0, e = 0 , s = 0, w = 0;

    w = mapfile->xref1;
    n = mapfile->yref1;
    pj_transform(pjsrc, pjtar, 1, 0, &w, &n, 0);

    e = mapfile->xref2;
    s = mapfile->yref2;
    pj_transform(pjsrc, pjtar, 1, 0, &e, &s, 0);

    if(w < westbound)   westbound = w;
    if(e > eastbound)   eastbound = e;
    if(n > northbound)  northbound = n;
    if(s < southbound)  southbound = s;

}


void CMapLevel::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = westbound;
    lat1 = northbound;
    lon2 = eastbound;
    lat2 = southbound;
}
