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


    qint16 dem[256 * 256];
    pBand->RasterIO(GF_Read,0,0,256,256,dem,256,256,GDT_CInt16,0,0);
    QImage img(256,256,QImage::Format_Indexed8);

    int i;
    QVector<QRgb> color;
    for(i = 0; i < 256; ++i){
        color << qRgba(i,i,i,20);
    }
    img.setColorTable(color);

    uchar * p = img.bits();
    for(i = 0; i < (256*256); i++){
        *p++ = (dem[1] >> 8);
    }

    img.save("dem.png");
}

CMapDEM::~CMapDEM()
{
    if(pjsrc) pj_free(pjsrc);
    if(pjtar) pj_free(pjtar);
    if(dataset) delete dataset;
}

float CMapDEM::getElevation(float& lon, float& lat)
{
    qint16 ele;
    double u = lon;
    double v = lat;

    pj_transform(pjtar, pjsrc, 1, 0, &u, &v, 0);

//     u *= RAD_TO_DEG;
//     v *= RAD_TO_DEG;

    int xoff = (u - xref1) / xscale;
    int yoff = (v - yref1) / yscale;

    CPLErr err = dataset->RasterIO(GF_Read, xoff, yoff, 1, 1, &ele, 1, 1, GDT_Int16, 1, 0, 0, 0, 0);
    if(err == CE_Failure){
        return WPT_NOFLOAT;
    }

    return (float)ele;
}
