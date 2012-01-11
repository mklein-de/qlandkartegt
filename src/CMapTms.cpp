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
#include "CDiskCache.h"
#include "CMainWindow.h"
#include "CDlgMapTmsConfig.h"

#include <QtGui>
#include <QtNetwork>

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
    copyright   = mapData.copyright;
    strUrl      = mapData.filename;

    pjsrc = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs");
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


    accessManager = new QNetworkAccessManager(this);
    accessManager->setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy));
    connect(accessManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(slotRequestFinished(QNetworkReply*)));

    diskCache = new CDiskCache(this);


    status = new QLabel(theMainWindow->getCanvas());
    theMainWindow->statusBar()->insertPermanentWidget(0,status);


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

    QImage img;
    int z           = 18 - zoomidx;
    lastTileLoaded  = false;

    pixBuffer.fill(Qt::white);
    QPainter p(&pixBuffer);


    double lon = x;
    double lat = y;
    convertM2Rad(lon, lat);

    lon *= RAD_TO_DEG;
    lat *= RAD_TO_DEG;


    int x1      = lon2tile(lon, z) / 256;
    int y1      = lat2tile(lat, z) / 256;
    double xx1  = tile2lon(x1, z) * DEG_TO_RAD;
    double yy1  = tile2lat(y1, z) * DEG_TO_RAD;
    convertRad2Pt(xx1, yy1);

    qDebug() << lon << lat << x1 << y1 << z << xx1 << yy1;

    int n = 0;
    int m = 0;

    double cx;
    double cy;

    do
    {
        do
        {
            double p1x = xx1 + n * 256;
            double p1y = yy1 + m * 256;
            double p2x = xx1 + (n + 1) * 256;
            double p2y = yy1 + (m + 1) * 256;

            cx = p2x;
            cy = p2y;

            convertPt2Rad(p1x, p1y);

            request_t req;
            req.url         = QUrl(strUrl.arg(z).arg(x1 + n).arg(y1 + m));
            req.lon         = p1x;
            req.lat         = p1y;
            req.zoomFactor  = zoomFactor;

            diskCache->restore(req.url.toString(), img);
            if(!img.isNull())
            {
                convertRad2Pt(req.lon,req.lat);
                p.drawImage(req.lon, req.lat,img);
            }
            else
            {
                addToQueue(req);
            }

            n++;
        }
        while(cx < rect.width());

        n = 0;
        m++;
    }
    while(cy < rect.height());

    checkQueue();
}

void CMapTms::addToQueue(request_t& req)
{
    newRequests.enqueue(req);
}

void CMapTms::checkQueue()
{
    if(newRequests.size() && pendRequests.size() < 6)
    {
        request_t req = newRequests.dequeue();

        if(diskCache->contains(req.url.toString()) || (req.zoomFactor != zoomFactor))
        {
            checkQueue();
            return;
        }

        QNetworkRequest request;
        request.setUrl(req.url);
        req.reply = accessManager->get(request);

        pendRequests[req.url.toString()] = req;
    }

    if(pendRequests.isEmpty() && newRequests.isEmpty())
    {
        status->setText(tr("Map loaded."));
        lastTileLoaded = true;
    }
    else
    {
        status->setText(tr("Wait for %1 tiles.").arg(pendRequests.size() + newRequests.size()));
    }


}


void CMapTms::slotRequestFinished(QNetworkReply* reply)
{
    QString _url_ = reply->url().toString();
    if(pendRequests.contains(_url_))
    {
        QImage img;
        QPainter p(&pixBuffer);

        request_t& req = pendRequests[_url_];

        // only take good responses
        if(!reply->error())
        {
            // read image data
            img.loadFromData(reply->readAll());
        }

        // always store image to cache, the cache will take care of NULL images
        diskCache->store(_url_, img);

        // only paint image if on current zoom factor
        if((req.zoomFactor == zoomFactor))
        {
            convertRad2Pt(req.lon, req.lat);
            p.drawImage(req.lon, req.lat, img);
        }

        // pending request finished
        pendRequests.remove(_url_);

    }

    // debug output any error
    if(reply->error())
    {
        qDebug() << reply->errorString();
    }

    // delete reply object
    reply->deleteLater();

    // check for more requests
    checkQueue();

    // the map did change
    emit sigChanged();
}

void CMapTms::config()
{
    CDlgMapTmsConfig dlg(*this);
    dlg.exec();
}

