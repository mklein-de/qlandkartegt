//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2009 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- acOSM_COMpanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------
#include <QApplication>
#include <QtNetwork/QHttp>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QtGui/QTransform>
#include <QThread>
#include <QDirIterator>
#include <math.h>
#include "COsmTilesHash.h"
#include <IMap.h>
#include "CMapOSM.h"
#ifndef M_PI
                                 /* pi */
# define M_PI   3.14159265358979323846
#endif

class COsmTilesHashCacheCleanup: public QThread
{
    public:
        COsmTilesHashCacheCleanup(QObject *p) : QThread(p) {
            QSettings cfg;

            if (!cfg.contains("osm/maxcachevalueMB"))
                cfg.setValue("osm/maxcachevalueMB",100);
            maxSizeInMB = cfg.value("osm/maxcachevalueMB").toInt();
            start(QThread::IdlePriority);
        };

        void run() {
            QString tempDir = QDir::tempPath() + "/qlandkarteqt/cache/";

            qint32 totalSize = 0;
            QFileInfoList fileList;
            QDirIterator it(tempDir, QStringList("*.png"), QDir::Files | QDir::Writable, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                totalSize += it.fileInfo().size();
                fileList << it.fileInfo();
            }
            qSort(fileList.begin(), fileList.end(), COsmTilesHashCacheCleanup::olderThan);

            qint32 currentSize = totalSize;
            foreach(QFileInfo fi, fileList) {
                if (currentSize < maxSizeInMB * 1024*1024)
                    break;
                QFile::remove ( fi.absoluteFilePath () );
                currentSize -= fi.size();
                // qDebug() << fi.lastRead() << fi.absoluteFilePath ();
            }
            qDebug() << QString("cache size before (%1) and after (%2) cleanup. maxcachevalueMB: %3").arg(totalSize).arg(currentSize).arg(maxSizeInMB);
        };

        static bool olderThan(const QFileInfo &fi1, const QFileInfo &fi2) {
            return fi1.lastRead() < fi2.lastRead();
        };
    private:
        qint32 maxSizeInMB;
};

COsmTilesHash::COsmTilesHash(CMapOSM *cmapOSM) : cmapOSM(cmapOSM)
{

    COsmTilesHashCacheCleanup *cleanup = new COsmTilesHashCacheCleanup(this);
    osmTileBaseUrl = "http://tile.openstreetmap.org/";
    getid = -1;
    bool enableProxy = false;
    requestInProgress =false;
    // enableProxy = CResources::self().getHttpProxy(osmTileBaseUrl,port);

    tilesConnection = new QHttp(this);
    tilesConnection->setHost("tile.openstreetmap.org");
    //  if(enableProxy) {
    //    tilesConnection->setProxy(url,port);
    //  }

    connect(tilesConnection,SIGNAL(requestFinished(int,bool)),this,SLOT(slotRequestFinished(int,bool)));

}


COsmTilesHash::~COsmTilesHash()
{

}


void COsmTilesHash::startNewDrawing( double lon, double lat, int osm_zoom, const QRect& window)
{
    this->osm_zoom=osm_zoom;
    this->window = window;
    tilesConnection->clearPendingRequests();
    osmRunningHash.clear();

    int osm_x_256 = long2tile(lon, osm_zoom);
    int osm_y_256 = lat2tile(lat, osm_zoom);

    int osm_x = osm_x_256 / (256 );
    int osm_y= osm_y_256 / (256 );

    int dx = osm_x_256 - (osm_x_256 / 256)*256.;
    int dy = osm_y_256 - (osm_y_256 / 256)*256.;

    QPoint point(-dx,-dy);

    int xCount = qMin((floorf((window.width() + dx) / 256.) ) + 1., floorf(pow(2.,osm_zoom))*1.);
    int yCount = qMin((floorf((window.height() + dy ) / 256.) ) + 1., floorf(pow(2.,osm_zoom))*1.) ;

    //qDebug() << xCount << yCount << window;

    image = QImage(window.size(),QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    for(int x=0; x<xCount; x++) {
        for (int y=0; y<yCount; y++) {
            QTransform t;
            t = t.translate(x*256,y*256);
            getImage(osm_zoom,osm_x+x,osm_y+y,t.map(point));
        }
    }
    emit newImageReady(image,!osmRunningHash.count());
}


void COsmTilesHash::getImage(int osm_zoom, int osm_x, int osm_y, QPoint point)
{
    // *  Tiles are 256  256 pixel PNG files
    // * Each zoom level is a directory, each column is a subdirectory, and each tile in that column is a file
    // * Filename(url) format is /zoom/x/y.png
    QString osmUrlPart = QString("/%1/%2/%3.png").arg(osm_zoom).arg(osm_x).arg(osm_y);
    QString osmFilePath = QDir::tempPath() + "/qlandkarteqt/cache/" + osmUrlPart;

    bool needHttpAction = true;
    bool outOfDate = false;
    if (tiles.contains(osmUrlPart)) {
        QPainter p(&image);
        p.drawImage(point,tiles.value(osmUrlPart));
#ifdef COSMTILESHASHDEBUG
        p.drawRect(QRect(point,QSize(255,255)));
        p.drawText(point + QPoint(10,10), "cached " + osmUrlPart);
#endif
        needHttpAction = false;
    }
    else if (QFileInfo(osmFilePath).exists()) {
        QFile f(osmFilePath);
        if (f.open(QIODevice::ReadOnly)) {
            QImage img1;
            img1.loadFromData(f.readAll());

            if(img1.format() != QImage::Format_Invalid) {
                QPainter p(&image);
                p.drawImage(point,img1);
                tiles.insert(osmUrlPart,img1);
                int days = QFileInfo(osmFilePath).lastModified().daysTo(QDateTime::currentDateTime());
                if ( days < 8) {
                    needHttpAction = false;
                }
                else {
                    outOfDate = true;
                    needHttpAction = true;
                    p.drawText(point + QPoint(10,256-10), tr("Tile %1 was loaded from %2 days old File. Reloading ...").arg(osmUrlPart).arg(days));
                }
            }
        }
    }
    if (needHttpAction && !osmRunningHash.contains(osmUrlPart)) {
        getid = tilesConnection->get(osmUrlPart);
        osmRunningHash.insert(osmUrlPart,getid);
        if (!outOfDate) {
            QPainter p(&image);
            //       p.drawText(point + QPoint(20,128), tr("Image is loading: %1").arg(osmUrlPart));
            //       p.drawText(point + QPoint(20,148), tr("%1 of %2 stored.").arg(tiles.count()).arg(getid));
        }
        startPointHash.insert(getid, point);
        osmUrlPartHash.insert(getid, osmUrlPart);
    }
}


void COsmTilesHash::slotRequestFinished(int id, bool error)
{
    if (error)
        return;

    if (!startPointHash.contains(id))
        return;

    // qDebug() << osmUrlPartHash.value(id) << id << error ;
    QImage img1;
    img1.loadFromData(tilesConnection->readAll());

    if(img1.format() == QImage::Format_Invalid) {
        // that:
        // link->setHost("tah.openstreetmap.org")
        // will cause a requestFinished() signal, too.
        // let's ignore it
        qDebug() << "QImage noc valid http";
        return;
    }
    QString osmUrlPart = osmUrlPartHash.value(id);
    QString filePath = QDir::tempPath() + "/qlandkarteqt/cache/" + osmUrlPart;

    QFileInfo fi(filePath);

    if( ! (fi.dir().exists()) )
        QDir().mkpath(fi.dir().path());

    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly)) {
        img1.save ( &f);
    }

    tiles.insert(osmUrlPart,img1);
    if (osmUrlPart.startsWith(QString("/%1/").arg(osm_zoom))) {
        QPainter p(&image);
        p.drawImage(startPointHash.value(id),img1);
#ifdef COSMTILESHASHDEBUG
        p.drawRect(QRect(startPointHash.value(id),QSize(255,255)));
        p.drawText(startPointHash.value(id) + QPoint(10,10), QString::number(id) + osmUrlPartHash.value(id));
#endif
        osmUrlPartHash.remove(id);
        startPointHash.remove(id);
        osmRunningHash.remove(osmUrlPart);
        emit newImageReady(image,!osmRunningHash.count());
    }
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
