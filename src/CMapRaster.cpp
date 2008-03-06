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

#include "CMapRaster.h"
#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include <QtGui>

CMapRaster::CMapRaster(const QString& fn, CCanvas * parent)
: IMap(parent)
, x(0)
, y(0)
, zoomlevel(1)
, zoomfactor(1.0)
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


    if(pBand->GetColorInterpretation() !=  GCI_PaletteIndex){
        delete dataset; dataset = 0;
        QMessageBox::warning(0, tr("Error..."), tr("File must be 8 bit palette."));
        return;
    }

    maparea.setWidth(dataset->GetRasterXSize());
    maparea.setHeight(dataset->GetRasterYSize());

    GDALColorTable * pct = pBand->GetColorTable();
    for(int i=0; i < pct->GetColorEntryCount(); ++i) {
        const GDALColorEntry& e = *pct->GetColorEntry(i);
        colortable << qRgba(e.c1, e.c2, e.c3, e.c4);
    }
}

CMapRaster::~CMapRaster()
{
    if(dataset) delete dataset;
}

void CMapRaster::convertPt2M(double& u, double& v)
{
    u = x + u * zoomfactor;
    v = y + v * zoomfactor;
}

void CMapRaster::convertM2Pt(double& u, double& v)
{
    u = (u - x) / zoomfactor;
    v = (v - y) / zoomfactor;
}

void CMapRaster::move(const QPoint& old, const QPoint& next)
{
    // move top left point by difference
    x += (old.x() - next.x()) * zoomfactor;
    y += (old.y() - next.y()) * zoomfactor;

}

void CMapRaster::zoom(bool zoomIn, const QPoint& p)
{
    double x1 = p.x();
    double y1 = p.y();


    convertPt2M(x1,y1);


    zoomlevel += zoomIn ? -1 : +1;
    if(zoomlevel < 1){
        zoomfactor  = 1.0 / - (zoomlevel - 2);
    }
    else{
        zoomfactor = zoomlevel;
    }

    convertM2Pt(x1,y1);
    move(QPoint(x1,y1),p);
}

void CMapRaster::zoom(double lon1, double lat1, double lon2, double lat2)
{
}

void CMapRaster::select(const QRect& rect)
{
}

void CMapRaster::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
}


void CMapRaster::draw(QPainter& p)
{
    if(!dataset) return;

    QRectF viewport(x, y, size.width() * zoomfactor,  size.height() *  zoomfactor);
    QRectF intersect = viewport.intersected(maparea);

    // x/y offset [pixel] into file matrix
    qint32 xoff = intersect.left();
    qint32 yoff = intersect.top();

    // number of x/y pixel to read
    qint32 pxx  = intersect.width();
    qint32 pxy  = intersect.height();

    // the final image width and height in pixel
    qint32 w    = (qint32)(pxx / zoomfactor) & 0xFFFFFFFC;
    qint32 h    = (qint32)(pxy / zoomfactor);

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
        double xx = (intersect.left() - x) / zoomfactor, yy = (intersect.top() - y)  / zoomfactor;
        p.drawPixmap(xx,yy,QPixmap::fromImage(img));
    }


    if(zoomfactor < 1.0) {
        QString str = tr("Overzoom x%1").arg(1/zoomfactor,0,'f',0);

        p.setPen(Qt::white);
        p.setFont(QFont("Sans Serif",14,QFont::Black));

        p.drawText(9 ,23, str);
        p.drawText(11,23, str);
        p.drawText(9 ,25, str);
        p.drawText(11,25, str);

        p.setPen(Qt::darkBlue);
        p.drawText(10,24,str);

    }
}


