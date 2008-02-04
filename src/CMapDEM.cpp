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

#include "CMapDEM.h"
#include "CWpt.h"

#include <QtGui>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <projects.h>

CMapDEM::CMapDEM(const QString& filename, QObject * parent)
    : QObject(parent)
    , filename(filename)
    , dataset(0)
    , xsize_px(0)
    , ysize_px(0)
    , pjsrc(0)
    , pjtar(0)
    , xscale(0.0)
    , yscale(0.0)
    , xref1(0.0)
    , yref1(0.0)
{
    dataset = (GDALDataset*)GDALOpen(filename.toUtf8(),GA_ReadOnly);
    if(dataset == 0) return;

    char str[1024];
    strncpy(str,dataset->GetProjectionRef(),sizeof(str));
    char * ptr = str;
    OGRSpatialReference oSRS;
    oSRS.importFromWkt(&ptr);
    oSRS.exportToProj4(&ptr);
    strProj = ptr;
    strProj = strProj.replace("+datum=potsdam","+nadgrids=./BETA2007.gsb");

    qDebug() << strProj;

    pjsrc = pj_init_plus(strProj.toLatin1());
    pjtar = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs");

    xsize_px = dataset->GetRasterXSize();
    ysize_px = dataset->GetRasterYSize();

    double adfGeoTransform[6];
    dataset->GetGeoTransform( adfGeoTransform );

    xscale  = adfGeoTransform[1];
    yscale  = adfGeoTransform[5];

    xref1   = adfGeoTransform[0];
    yref1   = adfGeoTransform[3];

    xref2   = xref1 + xsize_px * xscale;
    yref2   = yref1 + ysize_px * yscale;

    qDebug() << xref1 << yref1 << xref2 << yref2;

    GDALRasterBand * pBand;
    pBand = dataset->GetRasterBand(1);
    pBand->GetBlockSize(&tileWidth,&tileHeight);
}

CMapDEM::~CMapDEM()
{
    if(pjsrc) pj_free(pjsrc);
    if(pjtar) pj_free(pjtar);
    if(dataset) delete dataset;
}

float CMapDEM::getElevation(float& lon, float& lat)
{

    return 0;
}
