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

#include "CMapGeoTiff.h"

#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include <QtGui>

CMapGeoTiff::CMapGeoTiff(const QString& fn, CCanvas * parent)
: IMap("",parent)
, dataset(0)
, xsize_px(0)
, ysize_px(0)
, xscale(1.0)
, yscale(1.0)
, xref1(0)
, yref1(0)
, xref2(0)
, yref2(0)
, lon1(0)
, lat1(0)
, lon2(0)
, lat2(0)
, x(0)
, y(0)
, zoomFactor(1.0)
{
    filename = fn;

    dataset = (GDALDataset*)GDALOpen(filename.toUtf8(),GA_ReadOnly);
    if(dataset == 0){
        QMessageBox::warning(0, tr("Error..."), tr("Failed to load file: %1").arg(filename));
        return;
    }

    GDALRasterBand * pBand;
    pBand = dataset->GetRasterBand(1);
    if(pBand == 0){
        delete dataset; dataset = 0;
        QMessageBox::warning(0, tr("Error..."), tr("Failed to load file: %1").arg(filename));
        return;
    }


    if(pBand->GetColorInterpretation() !=  GCI_PaletteIndex && pBand->GetColorInterpretation() !=  GCI_GrayIndex){
        delete dataset; dataset = 0;
        QMessageBox::warning(0, tr("Error..."), tr("File must be 8 bit palette or gray indexed."));
        return;
    }

    if(pBand->GetColorInterpretation() ==  GCI_PaletteIndex ){
        GDALColorTable * pct = pBand->GetColorTable();
        for(int i=0; i < pct->GetColorEntryCount(); ++i) {
            const GDALColorEntry& e = *pct->GetColorEntry(i);
            colortable << qRgba(e.c1, e.c2, e.c3, e.c4);
        }
    }
    else if(pBand->GetColorInterpretation() ==  GCI_GrayIndex ){
        for(int i=0; i < 256; ++i) {
            colortable << qRgba(i, i, i, 255);
        }
    }

    int success = 0;
    double idx = pBand->GetNoDataValue(&success);

    if(success) {
        QColor tmp(colortable[idx]);
        tmp.setAlpha(0);
        colortable[idx] = tmp.rgba();
    }


    char str[1024];
    strncpy(str,dataset->GetProjectionRef(),sizeof(str));
    char * ptr = str;
    OGRSpatialReference oSRS;
    oSRS.importFromWkt(&ptr);
    oSRS.exportToProj4(&ptr);

    qDebug() << ptr;

    pjsrc = pj_init_plus(ptr);
    if(pjsrc == 0){
        delete dataset; dataset = 0;
        QMessageBox::warning(0, tr("Error..."), tr("No georeference information found."));
        return;
    }

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

    lon1 = xref1;
    lat1 = yref1;
    pj_transform(pjsrc,pjtar,1,0,&lon1,&lat1,0);

    lon2 = xref2;
    lat2 = yref2;
    pj_transform(pjsrc,pjtar,1,0,&lon2,&lat2,0);

    x = xref1;
    y = yref1;

    zoomidx = 1;
}

CMapGeoTiff::~CMapGeoTiff()
{
    if(pjsrc) pj_free(pjsrc);
    if(dataset) delete dataset;
}

void CMapGeoTiff::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    QRectF viewport  = QRectF(x, y, size.width() * xscale * zoomFactor,  size.height() * yscale * zoomFactor);
    QRectF maparea   = QRectF(QPointF(xref1, yref1), QPointF(xref2, yref2));
    QRectF intersect = viewport.intersected(maparea);

//     qDebug() << maparea << viewport << intersect;

    if(intersect.isValid()) {

        // x/y offset [pixel] into file matrix
        qint32 xoff = (intersect.left()   - xref1) / xscale;
        qint32 yoff = (intersect.bottom() - yref1) / yscale;

        // number of x/y pixel to read
        qint32 pxx  =   (qint32)(intersect.width()  / xscale);
        qint32 pxy  =  -(qint32)(intersect.height() / yscale);

        // the final image width and height in pixel
        qint32 w    =   (qint32)(pxx / zoomFactor) & 0xFFFFFFFC;
        qint32 h    =   (qint32)(pxy / zoomFactor);

        // correct pxx by truncation
        pxx         =   (qint32)(w * zoomFactor);

//         qDebug() << xoff << yoff << pxx << pxy << w << h;

        if(w != 0 && h != 0){

            GDALRasterBand * pBand;
            pBand = dataset->GetRasterBand(1);

            QImage img(QSize(w,h),QImage::Format_Indexed8);
            img.setColorTable(colortable);

            CPLErr err = pBand->RasterIO(GF_Read
                ,(int)xoff,(int)yoff
                ,pxx,pxy
                ,img.bits()
                ,w,h
                ,GDT_Byte,0,0);

            if(!err) {
                double xx = intersect.left(), yy = intersect.bottom();
                convertM2Pt(xx,yy);

//                 qDebug() << xx << yy;

                p.drawPixmap(xx,yy,QPixmap::fromImage(img));
            }
        }
    }


    QString str;
    if(zoomFactor < 1.0) {
        str = tr("Overzoom x%1").arg(1/zoomFactor,0,'f',0);
    }
    else{
        str = tr("Zoom level x%1").arg(zoomidx);
    }

    p.setPen(Qt::white);
    p.setFont(QFont("Sans Serif",14,QFont::Black));

    p.drawText(9  ,23, str);
    p.drawText(10 ,23, str);
    p.drawText(11 ,23, str);
    p.drawText(9  ,24, str);
    p.drawText(11 ,24, str);
    p.drawText(9  ,25, str);
    p.drawText(10 ,25, str);
    p.drawText(11 ,25, str);

    p.setPen(Qt::darkBlue);
    p.drawText(10,24,str);

}

void CMapGeoTiff::convertPt2M(double& u, double& v)
{
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;
}

void CMapGeoTiff::convertM2Pt(double& u, double& v)
{
    u = (u - x) / (xscale * zoomFactor);
    v = (v - y) / (yscale * zoomFactor);
}

void CMapGeoTiff::move(const QPoint& old, const QPoint& next)
{
    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference
    xx += old.x() - next.x();
    yy += old.y() - next.y();

    convertPt2M(xx,yy);
    x = xx;
    y = yy;
    emit sigChanged();
}

void CMapGeoTiff::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    zoomidx += zoomIn ? -1 : 1;
    zoom(zoomidx);

    // convert geo. coordinates back to point
    convertRad2Pt(p1.u, p1.v);

    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference point befor and after zoom
    xx += p1.u - p0.x();
    yy += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2M(xx, yy);
    x = xx;
    y = yy;

    emit sigChanged();
}

void CMapGeoTiff::zoom(qint32& level)
{
    // no level less than 1
    if(level < 1) {
        zoomFactor  = 1.0 / - (level - 2);
        qDebug() << "zoom:" << zoomFactor;
        return;
    }
    zoomFactor = level;
    emit sigChanged();
    qDebug() << "zoom:" << zoomFactor;
}

void CMapGeoTiff::zoom(double lon1, double lat1, double lon2, double lat2)
{

}

void CMapGeoTiff::select(const QRect& rect)
{
}

void CMapGeoTiff::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = this->lon1;
    lat1 = this->lat1;
    lon2 = this->lon2;
    lat2 = this->lat2;
}

