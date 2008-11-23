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
#include <assert.h>

#include "CMapDEM.h"
#include "CMapDB.h"
#include "CWpt.h"
#include "CStatusDEM.h"
#include "CMainWindow.h"

#include <gdal_priv.h>
#include <ogr_spatialref.h>

#ifdef WIN32
#include <float.h>
#define isnan(x) _isnan(x)
#endif

#include <QtGui>

CMapDEM::CMapDEM(const QString& filename, CCanvas * parent)
: IMap(eDEM, "",parent)
, weights(0)
, old_my_xscale(0)
, old_my_yscale(0)
, old_overlay(IMap::eNone)
{
    dataset = (GDALDataset*)GDALOpen(filename.toUtf8(),GA_ReadOnly);
    if(dataset == 0) {
        QMessageBox::warning(0, tr("Error..."), tr("Failed to load file: %1").arg(filename));
        return;
    }

    GDALRasterBand * pBand;
    pBand = dataset->GetRasterBand(1);
    if(pBand == 0) {
        delete dataset; dataset = 0;
        QMessageBox::warning(0, tr("Error..."), tr("Failed to load file: %1").arg(filename));
        return;
    }
    pBand->GetBlockSize(&tileWidth,&tileHeight);

    char str[1024];
    strncpy(str,dataset->GetProjectionRef(),sizeof(str));
    char * ptr = str;
    OGRSpatialReference oSRS;
    oSRS.importFromWkt(&ptr);
    oSRS.exportToProj4(&ptr);
    QString strProj = ptr;

    qDebug() << strProj;

    pjsrc = pj_init_plus(strProj.toLatin1());

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

    //     qDebug() << xref1 << yref1 << xref2 << yref2;

    int i;
    for(i = 0; i < 256; ++i) {
        graytable2 << qRgba(0,0,0,i);
    }

    for(i = 0; i < 128; ++i) {
        graytable1 << qRgba(0,0,0,(128 - i) << 1);
    }

    for(i = 128; i < 255; ++i) {
        graytable1 << qRgba(255,255,255,(i - 128) << 1);
    }

    status = new CStatusDEM(theMainWindow->getCanvas());
    theMainWindow->statusBar()->insertPermanentWidget(0,status);

    const int R = abs(yscale);
    const int C = abs(xscale);

    weights = new weight_t[R * C];

    for(int r=0; r < R; ++r) {
        for(int c=0; c < C; ++c) {
            weight_t& w = weights[(r * C) + c];

            //    p1            p2
            float _c = c; float c_ = C - c;

            //    p3            p4
            float _r = r; float r_ = R - r;

            //    p1                              p2
            float d1 = sqrt(_c*_c + _r*_r); float d2 = sqrt(c_*c_ + _r*_r);

            //    p3                              p4
            float d3 = sqrt(_c*_c + r_*r_); float d4 = sqrt(c_*c_ + r_*r_);

            w.c1 = pow(d1,-2);
            w.c2 = pow(d2,-2);
            w.c3 = pow(d3,-2);
            w.c4 = pow(d4,-2);

            float f = w.c1 + w.c2 + w.c3 + w.c4;

            w.c1 = w.c1 / f; if(isnan(w.c1)) w.c1 = 1.0;
            w.c2 = w.c2 / f; if(isnan(w.c2)) w.c2 = 1.0;
            w.c3 = w.c3 / f; if(isnan(w.c3)) w.c3 = 1.0;
            w.c4 = w.c4 / f; if(isnan(w.c4)) w.c4 = 1.0;

            //             qDebug() << r << c << "\t" << w.c1 << w.c2;
            //             qDebug() << "\t" << w.c3 << w.c4;
        }
    }
}


CMapDEM::~CMapDEM()
{
    if(pjsrc) pj_free(pjsrc);
    if(dataset) delete dataset;
    if(weights) delete [] weights;
    delete status;
}


void CMapDEM::convertPt2M(double& u, double& v)
{
}


void CMapDEM::convertM2Pt(double& u, double& v)
{
}


void CMapDEM::move(const QPoint& old, const QPoint& next)
{
}


void CMapDEM::zoom(bool zoomIn, const QPoint& p)
{
}


void CMapDEM::zoom(double lon1, double lat1, double lon2, double lat2)
{
}


void CMapDEM::zoom(qint32& level)
{
}


void CMapDEM::select(const QRect& rect)
{
}


void CMapDEM::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
}

void CMapDEM::getRegion(float *buffer, XY topLeft, XY bottomRight, int w, int h)
{
    int i, j;
    qint16 *region_data;
    int region_width;
    int region_height;

    if(pjsrc == 0) return;

    /*
        Calculate area of DEM data to be read.
    */
//    XY p1 = topLeft, p2 = bottomRight;
    // 1. convert top left and bottom right point into the projection system used by the DEM data
    pj_transform(pjtar, pjsrc, 1, 0, &topLeft.u, &topLeft.v, 0);
    pj_transform(pjtar, pjsrc, 1, 0, &bottomRight.u, &bottomRight.v, 0);

    // 2. get floating point offset of topleft corner
    double xoff1_f = (topLeft.u - xref1) / xscale;
    double yoff1_f = (topLeft.v - yref1) / yscale;

    // 3. truncate floating point offset into integer offset
    int xoff1 = xoff1_f;         //qDebug() << "xoff1:" << xoff1 << xoff1_f;
    int yoff1 = yoff1_f;         //qDebug() << "yoff1:" << yoff1 << yoff1_f;

    // 4. get floating point offset of bottom right corner
    double xoff2_f = (bottomRight.u - xref1) / xscale;
    double yoff2_f = (bottomRight.v - yref1) / yscale;

    // 5. round up (!) floating point offset into integer offset
    int xoff2 = ceil(xoff2_f);   //qDebug() << "xoff2:" << xoff2 << xoff2_f;
    if (xoff2 == (int) xoff2_f)
            xoff2 ++;
    int yoff2 = ceil(yoff2_f);   //qDebug() << "yoff2:" << yoff2 << yoff2_f;
    if (yoff2 == (int) yoff2_f)
            yoff2 ++;

    int w1 = xoff2 - xoff1;
    int h1 = yoff2 - yoff1;      //qDebug() << "w1:" << w1 << "h1:" << h1;

    // bail out if this is getting too big
    if(w1 > 10000 || h1 > 10000) return;

    int w2 = w, h2 = h;

    if (w > w1)
            w2= w1;
    if (h > h1)
            h2 = h1;

     if ((w2 != w) || (h2 != h)) {
            // we should increment values w2, because
            // want use widths 0 to w2 inclusive
            region_width = w2 + 1;
            w1++;
            region_height = h2 + 1;
            h1++;
     }

    region_data = new qint16[region_width * region_height];

    // read 16bit elevation data from GeoTiff
    GDALRasterBand * pBand;
    pBand = dataset->GetRasterBand(1);
    CPLErr err = pBand->RasterIO(GF_Read, xoff1, yoff1, w1, h1, region_data, region_width, region_height, GDT_Int16, 0, 0);
    if(err == CE_Failure) {
        qDebug() << "faillure" << endl;
        delete [] region_data;
        //FIXME add handle error
        return;
    }

#define GET_VALUE(X, Y)    region_data[region_width * (int)(Y) + (int)(X)]

    if ((w2 != w) || (h2 != h)) {
           // do interpolation if DEM data resolution less than required.
           double x, y, c, r;
           double xscale_my = (xoff2_f - xoff1_f) / (double)w;
           double yscale_my = (yoff2_f - yoff1_f) / (double)h;

           for (i = 0; i < w; i++) {
                   x = xoff1_f + xscale_my * i - xoff1;
                   c = x - (int) x;
                   c = c * abs(xscale);

                   for (j = 0; j < h; j++) {
                           y = yoff1_f + yscale_my * j - yoff1;
                           r = y - (int) y;
                           r = r * abs(yscale);
/*                         qDebug() << "wxh" << w << "x" << h << " " << w2 << "x" <<h2;
                           qDebug() << "scale " << xscale << " " << yscale;
                           qDebug() << "ij " << i << " " << j;
                           qDebug() << "xy" << x + xoff1 << " " << y + yoff1;
                           qDebug() << "cr" << c << " " << r;
*/
                           const weight_t& wt = weights[((int) r * (int)abs(xscale)) + (int) c];
                           buffer[i + j * w] = wt.c1 * GET_VALUE(x, y) + \
                                       wt.c2 * GET_VALUE(x + 1, y) + \
                                       wt.c3 * GET_VALUE(x, y + 1) + \
                                       wt.c4 * GET_VALUE(x + 1, y + 1);
/*                         qDebug() << buffer[i + j * w];
                           qDebug() << getElevation(p1.u + (p2.u - p1.u) / (double) w * i,
                                           p2.v + (p2.v - p1.v) / (double) h * j) <<  endl;
*/
                   }
           }
    } else {
            for (i = 0; i < w; i++)
                    for (j = 0; j < h; j++)
                            buffer[i + j * w] = region_data[i + j * w];
    }
    delete [] region_data;
}

float CMapDEM::getElevation(float lon, float lat)
{
    if(pjsrc == 0) return WPT_NOFLOAT;

    qint16 e[4];
    double u = lon;
    double v = lat;

    pj_transform(pjtar, pjsrc, 1, 0, &u, &v, 0);

    double xoff = (u - xref1) / xscale;
    double yoff = (v - yref1) / yscale;

    int c = (xoff - (int)xoff) * abs(xscale);
    int r = (yoff - (int)yoff) * abs(yscale);

    //     qDebug() << xoff << yoff << c << r;

    CPLErr err = dataset->RasterIO(GF_Read, xoff, yoff, 2, 2, &e, 2, 2, GDT_Int16, 1, 0, 0, 0, 0);
    if(err == CE_Failure) {
        return WPT_NOFLOAT;
    }

    const weight_t& w = weights[(r * (int)abs(xscale)) + c];

    float ele = w.c1 * e[0] + w.c2 * e[1] + w.c3 * e[2] + w.c4 * e[3];

    //     qDebug() << c << r << "\t" << w.c1 << e[0] << w.c2 << e[1];
    //     qDebug() << "\t"           << w.c3 << e[2] << w.c4 << e[3];

    return ele;
}


void CMapDEM::draw(QPainter& p)
{
    if(pjsrc == 0) return;

    draw();
    p.drawImage(0,0, buffer);
    //     qDebug() << "--------------------------";
}


void CMapDEM::draw()
{

    IMap::overlay_e overlay = status->getOverlayType();
    if(overlay == IMap::eNone){
        old_overlay = overlay;
        buffer.fill(Qt::transparent);
        return;
    }


    // check if old area matches new request
    // kind of a different way to calculate the needRedraw flag

    XY p1, p2;
    float my_xscale, my_yscale;

    getArea_n_Scaling_fromBase(p1, p2, my_xscale, my_yscale);

    if(    overlay == old_overlay
        && p1.u == old_p1.u && p1.v == old_p1.v
        && p2.u == old_p2.u && p2.v == old_p2.v
        && my_xscale == old_my_xscale && my_yscale == old_my_yscale
      )
    {
        return;
    }

    old_p1          = p1;
    old_p2          = p2;
    old_my_xscale   = my_xscale;
    old_my_yscale   = my_yscale;
    old_overlay     = overlay;

    buffer.fill(Qt::transparent);

    if(pjsrc == 0) return;

    /*
        Calculate area of DEM data to be read.
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
    int xoff1 = xoff1_f;         //qDebug() << "xoff1:" << xoff1 << xoff1_f;
    int yoff1 = yoff1_f;         //qDebug() << "yoff1:" << yoff1 << yoff1_f;

    // 4. get floating point offset of bottom right corner
    double xoff2_f = (_p2.u - xref1) / xscale;
    double yoff2_f = (_p2.v - yref1) / yscale;

    // 5. round up (!) floating point offset into integer offset
    int xoff2 = ceil(xoff2_f);   //qDebug() << "xoff2:" << xoff2 << xoff2_f;
    int yoff2 = ceil(yoff2_f);   //qDebug() << "yoff2:" << yoff2 << yoff2_f;

    /*
        The defined area into DEM data (xoff1,yoff1,xoff2,yoff2) covers a larger or equal
        area in world coordinates [m] than the current viewport.

        Next the width and height of the area in the DEM data and the corresponding sceen
        size is calculated.
    */

    /*
        Calculate the width and the height of the area. Extend width until it's a multiple of 4.
        This will be of advantag as QImage will process 32bit alligned bitmaps much faster.
    */
    int w1 = xoff2 - xoff1; while((w1 & 0x03) != 0) ++w1;
    int h1 = yoff2 - yoff1;      //qDebug() << "w1:" << w1 << "h1:" << h1;

    // bail out if this is getting too big
    if(w1 > 10000 || h1 > 10000) return;

    // now calculate the actual width and height of the viewport
    int w2 = w1 * xscale / my_xscale;
                                 //qDebug() << "w2:" << w2 << "h2:" << h2;
    int h2 = h1 * yscale / my_yscale;

    // as the first point off the DEM data will not match exactly the given top left corner
    // the bitmap has to be drawn with an offset.
    int pxx = (xoff1_f - xoff1) * xscale / my_xscale;
                                 //qDebug() << "pxx:" << pxx << "pxy:" << pxy;
    int pxy = (yoff1_f - yoff1) * yscale / my_yscale;

    // read 16bit elevation data from GeoTiff
    qint16 * data = new qint16[w1 * h1];

    GDALRasterBand * pBand;
    pBand = dataset->GetRasterBand(1);
    CPLErr err = pBand->RasterIO(GF_Read, xoff1, yoff1, w1, h1, data, w1, h1, GDT_Int16, 0, 0);
    if(err == CE_Failure) {
        delete [] data;
        return;
    }

    QImage img(w1,h1,QImage::Format_Indexed8);

    if(overlay == IMap::eShading) {
        shading(img,data);
    }
    else if(overlay == IMap::eContour) {
        contour(img,data);
    }
    else {
        qWarning() << "Unknown shading type";
        delete [] data;
        return;
    }

    delete [] data;

    // Finally scale the image to viewport size. QT will do the smoothing
    img = img.scaled(w2,h2, Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    QPainter p(&buffer);
    p.drawImage(-pxx, -pxy, img);
}


void CMapDEM::shading(QImage& img, qint16 * data)
{
    int i;
    int w1 = img.width();
    int h1 = img.height();
    // find minimum and maximum elevation within area
    int min = 32768;
    int max = -32768;
    int ele;

    for(i = 0; i < (w1 * h1); i++) {
        ele = data[i];
        if(ele < 0) continue;
        if(ele < min) min = ele;
        if(ele > max) max = ele;
    }

    /* Convert 16bit elevation data into 8 bit indices into a gray scale table.
       The dynamic will be 200 gray scales between min and max.
    */
    uchar * pixel = img.bits();
    img.setColorTable(graytable2);

    int f = (max -min);
    f = f ? f : 1;

    for(i = 0; i < ((w1 * h1) - 1); i++) {
        *pixel = ((data[i] - min) * 150 / f);
        ++pixel;
    }
}


void CMapDEM::contour(QImage& img, qint16 * data)
{
    int w1 = img.width();
    int h1 = img.height();

    int r,c,i;
    int diff = 0;
    int idx  = 0;
    int min  =  32768;
    int max  = -32768;
    for(r = 0; r < (h1 - 1); ++r) {
        for(c = 0; c < (w1 - 1); ++c) {
            diff  = data[idx +  1    ] - data[idx];
            diff += data[idx + w1    ] - data[idx];
            diff += data[idx + w1 + 1] - data[idx];
            data[idx++] = diff;
            if(diff < min) min = diff;
            if(diff > max) max = diff;
        }
        data[idx++] = 0;
    }
    for(c = 0; c < w1; ++c) {
        data[idx++] = 0;
    }

    int f = abs(min) < abs(max) ? abs(max) : abs(min);
    f = f ? f : 1;

    img.setColorTable(graytable1);
    uchar * pixel = img.bits();
    for(i = 0; i < (w1 * h1); ++i) {
        *pixel++ = 128 + data[i] * 100 / f;
    }
}
