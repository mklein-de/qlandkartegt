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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CMapOSM.h"
#include "GeoMath.h"
#include "CCanvas.h"

#include <QtGui>
#include "COsmTilesHash.h"
#include <QDebug>

CMapOSM::CMapOSM(CCanvas * parent)
: IMap(eRaster, "OSMTileServer", parent)
, xscale( 1.19432854652)
, yscale(-1.19432854652)
, x(0)
, y(0)
, zoomFactor(1.0)
{
    osmTiles = new COsmTilesHash(this);
    connect(osmTiles,SIGNAL(newImageReady(QImage,bool)),this,SLOT(newImageReady(QImage,bool)));
    pjsrc = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs");

    QSettings cfg;
    QString pos     = cfg.value("osm/topleft","").toString();
    zoomidx         = cfg.value("osm/zoomidx",1).toInt();

    lon1 = xref1   = -40075016/2;
    lat1 = yref1   =  40075016/2;
    lon2 = xref2   =  40075016/2;
    lat2 = yref2   = -40075016/2;
    pj_transform(pjsrc,pjtar,1,0,&lon1,&lat1,0);
    pj_transform(pjsrc,pjtar,1,0,&lon2,&lat2,0);

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
        pj_transform(pjtar,pjsrc,1,0,&x,&y,0);
    }

    zoom(zoomidx);

    resize(parent->size());
}

CMapOSM::~CMapOSM()
{
    QString pos;
    QSettings cfg;

    double u = x;
    double v = y;
    pj_transform(pjsrc,pjtar,1,0,&u,&v,0);

    GPS_Math_Deg_To_Str(u * RAD_TO_DEG, v * RAD_TO_DEG, pos);
    pos = pos.replace("\260","");

    cfg.setValue("osm/topleft",pos);
    cfg.setValue("osm/zoomidx",zoomidx);

    if(pjsrc) pj_free(pjsrc);
    if (osmTiles) delete osmTiles;
}

void CMapOSM::convertPt2M(double& u, double& v)
{
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;
}


void CMapOSM::convertM2Pt(double& u, double& v)
{
    u = (u - x) / (xscale * zoomFactor);
    v = (v - y) / (yscale * zoomFactor);
}


void CMapOSM::move(const QPoint& old, const QPoint& next)
{
    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference
    xx += old.x() - next.x();
    yy += old.y() - next.y();

    convertPt2M(xx,yy);
    x = xx;
    y = yy;
    needsRedraw = true;
    emit sigChanged();
}


void CMapOSM::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

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

    needsRedraw = true;
    blockSignals(false);
    emit sigChanged();
}


void CMapOSM::zoom(qint32& level)
{
    if(level > 17) level = 17;
    // no level less than 1
    if(level < 1) {
        level       = 1;
        zoomFactor  = 1.0;
        qDebug() << "zoom:" << zoomFactor;
        return;
    }
    zoomFactor = (1<<(level-1));
    //xscale = yscale = 2^(17-level);
    needsRedraw = true;
    emit sigChanged();

}


void CMapOSM::zoom(double lon1, double lat1, double lon2, double lat2)
{
    double u[3];
    double v[3];
    double dU, dV;
    int i;

    u[0] = lon1;
    v[0] = lat1;
    u[1] = lon2;
    v[1] = lat1;
    u[2] = lon1;
    v[2] = lat2;

    pj_transform(pjtar, pjsrc,3,0,u,v,0);
    dU = u[1] - u[0];
    dV = v[0] - v[2];

    int z1 = dU / size.width();
    int z2 = dV / size.height();

    for(i=0; i < 18; ++i){
        zoomFactor  = (1<<i);
        zoomidx     = i + 1;
        if(zoomFactor > z1 && zoomFactor > z2) break;
    }
//    zoomFactor = (z1 > z2 ? z1 : z2) + 1;

    double u_ = lon1 + (lon2 - lon1)/2;
    double v_ = lat1 + (lat2 - lat1)/2;
    convertRad2Pt(u_,v_);
    move(QPoint(u_,v_), rect.center());

    needsRedraw = true;
    emit sigChanged();

    qDebug() << "zoom:" << zoomFactor;
}

void CMapOSM::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    // render map if necessary
    if(needsRedraw) {
        draw();
    }

    p.drawImage(0,0,buffer);

    QString str;
    if(zoomFactor < 1.0) {
        str = tr("Overzoom x%1").arg(1/zoomFactor,0,'f',0);
    }
    else {
        str = tr("Zoom level x%1").arg(zoomFactor);
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

    p.setFont(QFont("Sans Serif",8,QFont::Black));
    CCanvas::drawText(tr("Created by OpenStreetMap under Creative Commons Attribution-ShareAlike 2.0 license"), p, rect.bottomLeft() + QPoint(rect.width() / 2, -5) , QColor(Qt::darkBlue));



}

void CMapOSM::draw()
{
    if(pjsrc == 0) return IMap::draw();

    int osm_zoom = 18 - zoomidx;

    double lon = x;
    double lat = y;
    convertM2Rad(lon,lat);
    lastTileLoaded = false;
    osmTiles->startNewDrawing( lon * RAD_TO_DEG, lat * RAD_TO_DEG,  osm_zoom, rect);

}

void CMapOSM::newImageReady(QImage image, bool lastTileLoaded)
{
  this->buffer = image;
  this->lastTileLoaded = true;
  needsRedraw = false;
//  doFastDraw = true;
 // qDebug() << "new image" << image.size() ;
  emit sigChanged();
}
