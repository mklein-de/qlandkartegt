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

#define M 25

// qint16 dem[256 * 256 * M * M];

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

    int i;
    for(i = 0; i < 256; ++i){
//         graytable << qRgba(0,0,0, 255 - i);
        graytable << qRgba(0,0,0,i);
    }
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

    int xoff = (u - xref1) / xscale;
    int yoff = (v - yref1) / yscale;

    CPLErr err = dataset->RasterIO(GF_Read, xoff, yoff, 1, 1, &ele, 1, 1, GDT_Int16, 1, 0, 0, 0, 0);
    if(err == CE_Failure){
        return WPT_NOFLOAT;
    }

    return (float)ele;
}


void CMapDEM::draw(QPainter& p, const XY& p1, const XY& p2, const float my_xscale, const float my_yscale)
{
    int i;
//     qDebug() << (p1.u * RAD_TO_DEG) << (p1.v * RAD_TO_DEG);
//     qDebug() << (p2.u * RAD_TO_DEG) << (p2.v * RAD_TO_DEG);
//     qDebug() << size;

    /*
        Calculate are into DEM data to be read.
    */

    XY _p1 = p1;
    XY _p2 = p2;

    // 1. convert top left and bottom right point into the projection system used by the DEM data
    pj_transform(pjtar, pjsrc, 1, 0, &_p1.u, &_p1.v, 0);
    pj_transform(pjtar, pjsrc, 1, 0, &_p2.u, &_p2.v, 0);

    // 2. get floating point offset of topleft corner
    double xoff1_f = (_p1.u - xref1) / xscale;
    double yoff1_f = (_p1.v - yref1) / yscale;

    // 3. truncate floating point offset into integer offset
    int xoff1 = xoff1_f; qDebug() << "xoff1:" << xoff1 << xoff1_f;
    int yoff1 = yoff1_f; qDebug() << "yoff1:" << yoff1 << yoff1_f;

    // 4. get floating point offset of bottom right corner
    double xoff2_f = (_p2.u - xref1) / xscale;
    double yoff2_f = (_p2.v - yref1) / yscale;

    // 5. round up (!) floating point offset into integer offset
    int xoff2 = ceil(xoff2_f); qDebug() << "xoff2:" << xoff2 << xoff2_f;
    int yoff2 = ceil(yoff2_f); qDebug() << "yoff2:" << yoff2 << yoff2_f;

    /*
        The defined area into DEM data (xoff1,yoff1,xoff2,yoff2) covers a larger or equal
        area in world coordinates [m] than the current viewport.

        Next the width and height of the area in the DEM data and the corresponding sceen
        size is calculated.
    */

    // calculate the width and the height of the are.
    //
    int w1 = xoff2 - xoff1; while((w1 & 0x03) != 0) ++w1;
    int h1 = yoff2 - yoff1; qDebug() << "w1:" << w1 << "h1:" << h1;

    if(w1 > 10000 || h1 > 10000) return;

    int w2 = w1 * xscale / my_xscale;
    int h2 = h1 * yscale / my_yscale; qDebug() << "w2:" << w2 << "h2:" << h2;

    GDALRasterBand * pBand;
    pBand = dataset->GetRasterBand(1);
    qint16 * data = new qint16[w1 * h1];
    CPLErr err = pBand->RasterIO(GF_Read, xoff1, yoff1, w1, h1, data, w1, h1, GDT_Int16, 0, 0);
    if(err == CE_Failure){
        delete [] data;
        return;
    }

    int min = 32768;
    int max = -32768;

    for(i = 0; i < (w1 * h1); i++){
        int ele = data[i];
        if(ele < 0) continue;
        if(ele < min) min = ele;
        if(ele > max) max = ele;
    }

    QImage img(w1,h1,QImage::Format_Indexed8);
    img.setColorTable(graytable);
    uchar * pixel = img.bits();
    for(i = 0; i < ((w1 * h1) - 1); i++){
        *pixel = ((data[i] - min) * 200 / (max -min));
        ++pixel;
    }

    img = img.scaled(w2,h2, Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    p.drawPixmap(0, 0, QPixmap::fromImage(img));

    delete [] data;
    qDebug() << "--------------------------";
}

