//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2011 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------

#include "IMap.h"
#include "CMapOSM.h"
#include "COsmTilesHash.h"
#include "CResources.h"
#include "version.h"

#include <QtGui>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkProxy>
#ifndef Q_OS_WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#endif

#ifndef M_PI
# define M_PI   3.14159265358979323846
#endif

COsmTilesHash::COsmTilesHash(QString tileUrl, QObject *parent)
    : QObject(parent)
{

    m_tileUrl = QUrl(tileUrl.startsWith("http://") ? tileUrl : "http://" + tileUrl);
    m_tilePath = m_tileUrl.path();

    diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(CResources::self().getPathMapCache().absolutePath());
    diskCache->setMaximumCacheSize(CResources::self().getSizeMapCache() * 1024*1024);

    m_networkAccessManager = new QNetworkAccessManager(this);
    m_networkAccessManager->setCache(diskCache);
    m_networkAccessManager->setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy));

    connect(m_networkAccessManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(slotRequestFinished(QNetworkReply*)));
	connect(m_networkAccessManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), 
			this, SLOT(slotProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));		
}


COsmTilesHash::~COsmTilesHash()
{
    QSettings cfg;
    cfg.setValue("tms/maxcachevalueMB",100);
}



void COsmTilesHash::startNewDrawing( double lon, double lat, int osm_zoom, const QRect& window)
{
    int osm_x_256 = long2tile(lon, osm_zoom);
    int osm_y_256 = lat2tile(lat, osm_zoom);

    int osm_x = osm_x_256 / (256 );
    int osm_y= osm_y_256 / (256 );

    int dx = osm_x_256 - (osm_x_256 / 256)*256.;
    int dy = osm_y_256 - (osm_y_256 / 256)*256.;

    QPoint point(-dx,-dy);

    int xCount = qMin((floorf((window.width() + dx) / 256.) ) + 1., floorf(pow(2.,osm_zoom))*1.);
    int yCount = qMin((floorf((window.height() + dy ) / 256.) ) + 1., floorf(pow(2.,osm_zoom))*1.) ;

    pixmap = QPixmap(window.size());
    pixmap.fill(Qt::white);

    m_queuedRequests.clear();

    foreach(QString url, m_activeRequests.keys())
    {
        m_activeRequests.insert(url, QPoint());
    }

    for(int x=0; x<xCount; x++)
    {
        for (int y=0; y<yCount; y++)
        {
            QTransform t;
            t = t.translate(x*256,y*256);
            getImage(osm_zoom,osm_x+x,osm_y+y,t.map(point));
        }
    }

    emit newImageReady(pixmap,!m_activeRequests.count());
}

void COsmTilesHash::getImage(int osm_zoom, int osm_x, int osm_y, QPoint point)
{
    // *  Tiles are 256  256 pixel PNG files
    // * Each zoom level is a directory, each column is a subdirectory, and each tile in that column is a file
    // * Filename(url) format is /zoom/x/y.png
    m_tileUrl.setPath(QString(m_tilePath).arg(osm_zoom).arg(osm_x).arg(osm_y));

    if(m_activeRequests.contains(m_tileUrl.toString()))
    {
        m_activeRequests.insert(m_tileUrl.toString(),point);
        return;
    }
    if(m_tileHash.contains(m_tileUrl.toString()))
    {
        QPainter p(&pixmap);
        p.drawPixmap(point,m_tileHash.value(m_tileUrl.toString()));
        return;
    }
    QNetworkRequest request;
    request.setUrl(m_tileUrl);
    m_queuedRequests.enqueue(qMakePair(request,point));
    dequeue();
}

void COsmTilesHash::dequeue()
{
     if(m_queuedRequests.size() && m_activeRequests.size() < 6)
     {
         QPair<QNetworkRequest, QPoint > pair = m_queuedRequests.dequeue();
         QNetworkReply *reply = m_networkAccessManager->get(pair.first);
         m_activeRequests.insert(reply->url().toString(),pair.second);
     }
}

void COsmTilesHash::slotRequestFinished(QNetworkReply* reply)
{

    QPoint startPoint = m_activeRequests.value(reply->url().toString(), QPoint());
    m_activeRequests.remove(reply->url().toString());
    dequeue();
    if (reply->error() != QNetworkReply::NoError)
    {
        //qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }


    QPixmap img1;
    img1.loadFromData(reply->readAll());

    if(img1.isNull())
    {
        // that:
        // link->setHost("tah.openstreetmap.org")
        // will cause a requestFinished() signal, too.
        // let's ignore it
        qDebug() << tr("The recieved data is not an valid image. Maybe it isn't an image ...");
    }
    else
    {
        m_tileHash.insert(reply->url().toString(),img1);
        if(!startPoint.isNull())
        {
            QPainter p(&pixmap);
            p.drawPixmap(startPoint,img1);
            emit newImageReady(pixmap,m_activeRequests.isEmpty());
        }
    }
    reply->deleteLater();
    return;
}


int COsmTilesHash::long2tile(double lon, int z)
{
    return (int)(qRound(256*(lon + 180.0) / 360.0 * pow(2.0, z)));
}


int COsmTilesHash::lat2tile(double lat, int z)
{
    return (int)(qRound(256*(1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
}


void COsmTilesHash::slotProxyAuthenticationRequired(const QNetworkProxy &prox, QAuthenticator *auth)
{
	QString user;
    QString pwd;
	
    CResources::self().getHttpProxyAuth(user,pwd);
	
	auth->setUser(user);
	auth->setPassword(pwd);
}	 