/*********************************************************************************************
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CMapWMS.h"
#include "CWMSResponse.h"
#include "GeoMath.h"

#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <QtGui>

CMapWMS::CMapWMS(const QString& key, const QString& fn, CCanvas * parent)
: IMap(eRaster, key, parent)
, x(0)
, y(0)
, zoomFactor(1.0)
{
    filename = fn;

    dataset = (GDALDataset*)GDALOpen(filename.toLocal8Bit(),GA_ReadOnly);
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

    char str[1024];
    strncpy(str,dataset->GetProjectionRef(),sizeof(str));
    char * ptr = str;
    OGRSpatialReference oSRS;
    oSRS.importFromWkt(&ptr);
    oSRS.exportToProj4(&ptr);

    qDebug() << ptr;

    pjsrc = pj_init_plus(ptr);
    if(pjsrc == 0) {
        delete dataset; dataset = 0;
        QMessageBox::warning(0, tr("Error..."), tr("No georeference information found."));
        return;
    }

    xsize_px = dataset->GetRasterXSize();
    ysize_px = dataset->GetRasterYSize();

    double adfGeoTransform[6];
    dataset->GetGeoTransform( adfGeoTransform );

    xscale  = adfGeoTransform[1] * DEG_TO_RAD;
    yscale  = adfGeoTransform[5] * DEG_TO_RAD;

    xref1   = adfGeoTransform[0] * DEG_TO_RAD;
    yref1   = adfGeoTransform[3] * DEG_TO_RAD;

    xref2   = xref1 + xsize_px * xscale;
    yref2   = yref1 + ysize_px * yscale;

    lon1 = xref1;
    lat1 = yref1;
    pj_transform(pjsrc,pjtar,1,0,&lon1,&lat1,0);

    lon2 = xref2;
    lat2 = yref2;
    pj_transform(pjsrc,pjtar,1,0,&lon2,&lat2,0);

    x = xref1;
    y = yref1;

    zoomidx = 1;

    QSettings cfg;
    cfg.beginGroup("wms/maps");
    cfg.beginGroup(filename);
    QString pos     = cfg.value("topleft","").toString();
    zoomidx         = cfg.value("zoomidx",1).toInt();
    cfg.endGroup();
    cfg.endGroup();

    if(pos.isEmpty()) {
        x = 0;
        y = 0;
    }
    else {
        float u = 0;
        float v = 0;
        GPS_Math_Str_To_Deg(pos, u, v);
        x = u * DEG_TO_RAD;
        y = v * DEG_TO_RAD;
    }
    zoom(zoomidx);

}


CMapWMS::~CMapWMS()
{
    QString pos;
    QSettings cfg;
    cfg.beginGroup("wms/maps");
    cfg.beginGroup(filename);
    GPS_Math_Deg_To_Str(x * RAD_TO_DEG, y * RAD_TO_DEG, pos);
    pos = pos.replace("\260","");
    cfg.setValue("topleft",pos);
    cfg.setValue("zoomidx",zoomidx);
    cfg.endGroup();
    cfg.endGroup();


    if(pjsrc) pj_free(pjsrc);

    if(dataset) delete dataset;
}


void CMapWMS::convertPt2M(double& u, double& v)
{
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;
}


void CMapWMS::convertM2Pt(double& u, double& v)
{
    u = (u - x) / (xscale * zoomFactor);
    v = (v - y) / (yscale * zoomFactor);
}


void CMapWMS::move(const QPoint& old, const QPoint& next)
{
    needsRedraw = true;

    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference
    xx += old.x() - next.x();
    yy += old.y() - next.y();

    convertPt2M(xx,yy);
    x = xx;
    y = yy;
    setFastDraw();
    emit sigChanged();
}


void CMapWMS::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

    needsRedraw = true;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    zoomidx += zoomIn ? -1 : 1;
    // sigChanged will be sent at the end of this function
    blockSignals(true);
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
    blockSignals(false);
    emit sigChanged();
}


void CMapWMS::zoom(qint32& level)
{
    needsRedraw = true;

    // no level less than 1
    if(level < 1) {
        zoomFactor  = 1.0 / - (level - 2);
        qDebug() << "zoom:" << zoomFactor;
        return;
    }
    zoomFactor = level;
    setFastDraw();
    emit sigChanged();
    qDebug() << "zoom:" << zoomFactor;
}


void CMapWMS::zoom(double lon1, double lat1, double lon2, double lat2)
{
    double u[3];
    double v[3];
    double dU, dV;

    needsRedraw = true;

    u[0] = lon1;
    v[0] = lat1;
    u[1] = lon2;
    v[1] = lat1;
    u[2] = lon1;
    v[2] = lat2;

    pj_transform(pjtar, pjsrc,3,0,u,v,0);
    dU = (u[1] - u[0]) / xscale;
    dV = (v[0] - v[2]) / yscale;

    int z1 = dU / size.width();
    int z2 = dV / size.height();

    zoomFactor = (z1 > z2 ? z1 : z2)  + 1;

    double u_ = lon1 + (lon2 - lon1)/2;
    double v_ = lat1 + (lat2 - lat1)/2;
    convertRad2Pt(u_,v_);
    move(QPoint(u_,v_), rect.center());

    emit sigChanged();

    qDebug() << "zoom:" << zoomFactor;
}


void CMapWMS::select(const QRect& rect)
{
}


void CMapWMS::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = this->lon1;
    lat1 = this->lat1;
    lon2 = this->lon2;
    lat2 = this->lat2;
}


void CMapWMS::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    if(needsRedraw){
        draw();
    }

    QString str;
    if(zoomFactor < 1.0) {
        str = tr("Overzoom x%1").arg(1/zoomFactor,0,'f',0);
    }
    else {
        str = tr("Zoom level x%1").arg(zoomidx);
    }

    p.drawImage(0,0,buffer);

        // render overlay
    if(!ovlMap.isNull() && !doFastDraw){
        ovlMap->draw(size, needsRedraw, p);
    }

    needsRedraw = false;

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


    if(doFastDraw) setFastDraw();

}


void CMapWMS::draw()
{
    if(pjsrc == 0) return IMap::draw();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QVector<QRgb> graytable2;
    int i;
    for(i = 0; i < 256; ++i) {
        graytable2 << qRgb(i,i,i);
    }


    buffer.fill(Qt::white);
    QPainter _p_(&buffer);

    QRectF viewport  = QRectF(x, y, size.width() * xscale * zoomFactor,  size.height() * yscale * zoomFactor);
    QRectF maparea   = QRectF(QPointF(xref1, yref1), QPointF(xref2, yref2));
    QRectF intersect = viewport.intersected(maparea);

    qDebug() << maparea << viewport << intersect;

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

        qDebug() << xoff << yoff << pxx << pxy << w << h;

        if(w != 0 && h != 0) {

            QImage img(QSize(w,h),QImage::Format_RGB32);
//             QImage img(QSize(w,h*12),QImage::Format_Indexed8);
//             img.setColorTable(graytable2);
            img.fill(0);

            QByteArray data(w*h*3,128);


            CPLErr err = dataset->RasterIO(GF_Read
                ,(int)xoff,(int)yoff
                ,pxx,pxy
                ,data.data()
                ,w,h
                ,GDT_Byte,3,NULL,0,0,0);

            quint8 * pR     = (quint8 *)data.data();
            quint8 * pG     = (quint8 *)data.data() + w* h;
            quint8 * pB     = (quint8 *)data.data() + w* h + w * h;
            quint32 * pImg  = (quint32 *)img.bits();

            for(i = 0; i < w*h; i++){
                *pImg++ = qRgb(*pR++,*pG++,*pB++);
            }

            qDebug() << err;
            if(!err) {
                double xx = intersect.left(), yy = intersect.bottom();
                convertM2Pt(xx,yy);
                _p_.drawImage(xx,yy,img);
            }
            else{
                IMap::draw();
            }
        }
    }

    QApplication::restoreOverrideCursor();
}


void CMapWMS::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{
    QRectF r(x, y, size.width() * xscale * zoomFactor,  size.height() * yscale * zoomFactor);
    p1.u        = r.left();
    p1.v        = r.top();
    p2.u        = r.right();
    p2.v        = r.bottom();

    my_xscale   = xscale * zoomFactor;
    my_yscale   = yscale * zoomFactor;
}