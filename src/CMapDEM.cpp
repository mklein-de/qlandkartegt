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


void CMapDEM::draw(QPainter& p, const XY& p1, const XY& p2, const QSize& size)
{
    int i;
//     qDebug() << (p1.u * RAD_TO_DEG) << (p1.v * RAD_TO_DEG);
//     qDebug() << (p2.u * RAD_TO_DEG) << (p2.v * RAD_TO_DEG);
//     qDebug() << size;

    XY _p1 = p1;
    XY _p2 = p2;

    pj_transform(pjtar, pjsrc, 1, 0, &_p1.u, &_p1.v, 0);
    pj_transform(pjtar, pjsrc, 1, 0, &_p2.u, &_p2.v, 0);

//     qDebug() << _p1.u << _p1.v << _p2.u << _p2.v;

    double fx = (_p2.u - _p1.u) / size.width();
    double fy = (_p1.v - _p2.v) / size.height();

//     qDebug() << fx << fy;

    double f_xoff = (_p1.u - xref1) / xscale;
    double f_yoff = (_p1.v - yref1) / yscale;

    int xoff = f_xoff;
    int yoff = f_yoff;

    f_xoff -= xoff;
    f_yoff -= yoff;

//     qDebug() << "1:" << fx  << fy;
//     qDebug() << "2:" << (int)(f_xoff * xscale / fx)  << (int)(f_yoff * yscale / fy);

    int w1 =  (_p2.u - _p1.u) / xscale;
    int h1 =  (_p2.v - _p1.v) / yscale;

    if(w1 > 1000 || h1 > 1000) return;

    int w2 = size.width() & 0xFFFFFFFC;
    int h2 = size.height();

//     qDebug() << ((double)w2/w1) << ((double)h2/h1);

    GDALRasterBand * pBand;
    pBand = dataset->GetRasterBand(1);

    QImage img(w2,h2,QImage::Format_Indexed8);
    img.setColorTable(graytable);
    img.fill(255);

    qint16 * data = new qint16[w2 * h2];
//     qDebug() << xoff1 << yoff1 << w1 << h1 << w2 << h2;
    CPLErr err = pBand->RasterIO(GF_Read, xoff, yoff, w1, h1, data, w2, h2, GDT_Int16, 0, 0);
    if(err == CE_Failure){
        delete [] data;
        return;
    }

    int min = 32768;
    int max = -32768;

    for(i = 0; i < ((w2 * h2) - 1); i++){
        int ele = data[i];
        if(ele < 0) continue;
        if(ele < min) min = ele;
        if(ele > max) max = ele;
    }


    uchar * pixel = img.bits();
    for(i = 0; i < ((w2 * h2) - 1); i++){
        *pixel = ((data[i] - min) * 200 / (max -min));
        ++pixel;
    }

    p.drawPixmap(-((f_xoff * xscale + xscale )/ fx), ((f_yoff * yscale + yscale) / fy), QPixmap::fromImage(img));

//     img.save("xxx.png");

    delete [] data;
}
