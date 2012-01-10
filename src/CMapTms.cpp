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

#include "CMapTms.h"
#include "CMapDB.h"
#include "GeoMath.h"

#include <QtGui>

CMapTms::CMapTms(const QString &key, const QString &filename, CCanvas *parent)
: IMap(eTMS,key,parent)
, zoomFactor(1.0)
, x(0)
, y(0)
, xscale( 1.19432854652)
, yscale(-1.19432854652)
, needsRedrawOvl(true)
, lastTileLoaded(false)
{
    QSettings cfg;

    CMapDB::map_t mapData = CMapDB::self().getMapData(key);
    copyright = mapData.copyright;

    pjsrc = pj_init_plus("+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +a=6378137 +b=6378137 +units=m +no_defs ");
    oSRS.importFromProj4(getProjection());

    char * ptr = pj_get_def(pjsrc,0);
    qDebug() << "tms:" << ptr;

    QString pos = cfg.value("tms/topleft","N82 58.759 W151 08.934").toString();


    x       = cfg.value("tms/lon", 12.098133).toDouble() * DEG_TO_RAD;
    y       = cfg.value("tms/lat", 49.019233).toDouble() * DEG_TO_RAD;
    zoomidx = cfg.value("tms/zoomidx",15).toInt();

    lon1 = xref1   = -40075016/2;
    lat1 = yref1   =  40075016/2;
    lon2 = xref2   =  40075016/2;
    lat2 = yref2   = -40075016/2;
    pj_transform(pjsrc,pjtar,1,0,&lon1,&lat1,0);
    pj_transform(pjsrc,pjtar,1,0,&lon2,&lat2,0);

    zoom(zoomidx);
}

CMapTms::~CMapTms()
{
    QString pos;
    QSettings cfg;

    cfg.setValue("tms/lon", x * RAD_TO_DEG);
    cfg.setValue("tms/lat", y * RAD_TO_DEG);
    cfg.setValue("tms/zoomidx",zoomidx);

    if(pjsrc) pj_free(pjsrc);
}


void CMapTms::convertPt2M(double& u, double& v)
{

}

void CMapTms::convertM2Pt(double& u, double& v)
{

}

void CMapTms::convertPt2Pixel(double& u, double& v)
{

}

void CMapTms::move(const QPoint& old, const QPoint& next)
{

}

void CMapTms::zoom(bool zoomIn, const QPoint& p)
{

}

void CMapTms::zoom(double lon1, double lat1, double lon2, double lat2)
{

}

void CMapTms::zoom(qint32& level)
{

}

void CMapTms::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{

}


void CMapTms::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{

}

