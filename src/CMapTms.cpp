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
#include "CResources.h"

#include <QtGui>

CMapTms::CMapTms(const QString& key, CCanvas *parent)
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
    u = x + u * xscale * zoomFactor;
    v = y + v * yscale * zoomFactor;
}

void CMapTms::convertM2Pt(double& u, double& v)
{
    u = (u - x) / (xscale * zoomFactor);
    v = (v - y) / (yscale * zoomFactor);
}


void CMapTms::move(const QPoint& old, const QPoint& next)
{
    double xx = x, yy = y;
    convertM2Pt(xx, yy);

    // move top left point by difference
    xx += old.x() - next.x();
    yy += old.y() - next.y();

    convertPt2M(xx,yy);
    x = xx;
    y = yy;
    needsRedraw     = true;
    needsRedrawOvl  = true;

    setAngleNorth();
    emit sigChanged();
}

void CMapTms::zoom(bool zoomIn, const QPoint& p0)
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

    needsRedraw     = true;
    needsRedrawOvl  = true;
    blockSignals(false);
    emit sigChanged();
}

void CMapTms::zoom(double lon1, double lat1, double lon2, double lat2)
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
    dU = fabs(u[1] - u[0]);
    dV = fabs(v[0] - v[2]);

    int z1 = dU / size.width();
    int z2 = dV / size.height();

    for(i=0; i < 18; ++i)
    {
        zoomFactor  = (1<<i);
        zoomidx     = i + 1;
        if(zoomFactor > z1 && zoomFactor > z2) break;
    }

    qDebug() << zoomFactor << z1 << zoomFactor << z2;

    double u_ = lon1 + (lon2 - lon1)/2;
    double v_ = lat1 + (lat2 - lat1)/2;
    convertRad2Pt(u_,v_);
    move(QPoint(u_,v_), rect.center());

    needsRedraw     = true;
    needsRedrawOvl  = true;
    emit sigChanged();

    qDebug() << "zoom:" << zoomFactor;
}

void CMapTms::zoom(qint32& level)
{
    if(level > 18) level = 18;
    // no level less than 1
    if(level < 1)
    {
        level       = 1;
        zoomFactor  = 1.0;
        qDebug() << "zoom:" << zoomFactor;
        return;
    }
    zoomFactor = (1<<(level-1));
    needsRedraw     = true;
    needsRedrawOvl  = true;

    emit sigChanged();
}

void CMapTms::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = this->lon1;
    lat1 = this->lat1;
    lon2 = this->lon2;
    lat2 = this->lat2;
}


void CMapTms::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{
    p1.u        = 0;
    p1.v        = 0;
    p2.u        = rect.width();
    p2.v        = rect.height();

    convertPt2Rad(p1.u, p1.v);
    convertPt2Rad(p2.u, p2.v);

    my_xscale   = xscale * zoomFactor;
    my_yscale   = yscale * zoomFactor;
}


void CMapTms::draw(QPainter& p)
{
    if(pjsrc == 0) return IMap::draw(p);

    // render map if necessary
    if(needsRedraw)
    {
        draw();
    }

    p.drawPixmap(0,0,pixBuffer);

    // render overlay
    if(!ovlMap.isNull() && lastTileLoaded && !doFastDraw)
    {
        ovlMap->draw(size, needsRedrawOvl, p);
        needsRedrawOvl = false;
    }

    needsRedraw = false;

    if(CResources::self().showZoomLevel())
    {

        QString str;
        if(zoomFactor < 1.0)
        {
            str = tr("Overzoom x%1").arg(1/zoomFactor,0,'f',0);
        }
        else
        {
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
    }

    p.setFont(QFont("Sans Serif",8,QFont::Black));
    CCanvas::drawText(tr("%1 %2").arg(QChar(0x00A9)).arg(copyright), p, rect.bottomLeft() + QPoint(rect.width() / 2, -5) , QColor(Qt::darkBlue));
}

void CMapTms::draw()
{
    if(pjsrc == 0) return IMap::draw();

    int tms_zoom    = 18 - zoomidx;
    lastTileLoaded  = false;

    pixBuffer.fill(Qt::white);
    QPainter p(&pixBuffer);

    double lon      = x;
    double lat      = y;
    convertM2Rad(lon,lat);

    int tms_x_256   = lon2tile(lon, tms_zoom);
    int tms_y_256   = lat2tile(lat, tms_zoom);
    int tms_x       = tms_x_256 / 256 ;
    int tms_y       = tms_y_256 / 256 ;
    int dx          = tms_x_256 - (tms_x_256 / 256) * 256.;
    int dy          = tms_y_256 - (tms_y_256 / 256) * 256.;


    qDebug() << tms_x_256 << tms_x << dx;

    int xCount      = qMin((floorf((rect.width() + dx)   / 256.) ) + 1.0, floorf(pow(2.0,tms_zoom))*1.0);
    int yCount      = qMin((floorf((rect.height() + dy ) / 256.) ) + 1.0, floorf(pow(2.0,tms_zoom))*1.0) ;


    for(int x=0; x<xCount; x++)
    {
        for (int y=0; y<yCount; y++)
        {
//            getImage(osm_zoom,osm_x+x,osm_y+y,t.map(point));

        }
    }

}



//void COsmTilesHash::startNewDrawing( double lon, double lat, int osm_zoom, const QRect& window)
//{
//    int osm_x_256 = long2tile(lon, osm_zoom);
//    int osm_y_256 = lat2tile(lat, osm_zoom);

//    int osm_x = osm_x_256 / (256 );
//    int osm_y= osm_y_256 / (256 );

//    int dx = osm_x_256 - (osm_x_256 / 256)*256.;
//    int dy = osm_y_256 - (osm_y_256 / 256)*256.;

//    QPoint point(-dx,-dy);

//    int xCount = qMin((floorf((window.width() + dx) / 256.) ) + 1., floorf(pow(2.,osm_zoom))*1.);
//    int yCount = qMin((floorf((window.height() + dy ) / 256.) ) + 1., floorf(pow(2.,osm_zoom))*1.) ;

//    pixmap = QPixmap(window.size());
//    pixmap.fill(Qt::white);

//    m_queuedRequests.clear();

//    foreach(QString url, m_activeRequests.keys())
//    {
//        m_activeRequests.insert(url, QPoint());
//    }

//    for(int x=0; x<xCount; x++)
//    {
//        for (int y=0; y<yCount; y++)
//        {
//            QTransform t;
//            t = t.translate(x*256,y*256);
//            getImage(osm_zoom,osm_x+x,osm_y+y,t.map(point));
//        }
//    }

//    emit newImageReady(pixmap,!m_activeRequests.count());
//}

