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
#include <QDateTime>
#include <QtGui/QTransform>
#include <math.h>
#include "COsmTilesHash.h"
#include <IMap.h>
#include "CMapOSM.h"
COsmTilesHash::COsmTilesHash(CMapOSM *cmapOSM) : cmapOSM(cmapOSM) {
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

COsmTilesHash::~COsmTilesHash() {

}

void COsmTilesHash::startNewDrawing( double lon, double lat, int x, int y, int osm_zoom, const QRect& window)
{
  if (x == this->x && y == this->y && osm_zoom == this ->osm_zoom && window == this->window)
  {
    return;
  }

  this->x = x;
  this->y = y;
  this->osm_zoom=osm_zoom;
  this->window = window;
  tilesConnection->clearPendingRequests();
  osmRunningHash.clear();
  //startPointHash.clear();
  //osmUrlPartHash.clear();

 // qDebug() << "zoom" << osm_zoom;

  int osm_x_256 = long2tile(lon, osm_zoom);
  int osm_y_256 = lat2tile(lat, osm_zoom);

  int osm_x = osm_x_256 / (256 );
  int osm_y= osm_y_256 / (256 );

  int dx = osm_x_256 - (osm_x_256 / 256)*256.;
  int dy = osm_y_256 - (osm_y_256 / 256)*256.;

  QPoint point(-dx,-dy);


  double osm_lon = tile2long(osm_x, osm_zoom);
  double osm_lat = tile2lat(osm_y, osm_zoom);

  int xCount = qMin(floor((window.width() / 256)) + 1, pow(2,osm_zoom)+1);
  int yCount = qMin(floor((window.height() / 256)) + 1, pow(2,osm_zoom)+1);
  //QPoint point((-osm_x_delta*256),(-osm_y_delta*256));

 //QPoint point= cmapOSM->offSetInPixel(osm_lon, osm_lat);

 // qDebug() << point << osm_lon << osm_lat << osm_x << osm_y << osm_lon << osm_lat << osm_x_256 << osm_y_256;
 // qDebug() << "count: "<< xCount << yCount;
//  image.fill(0);
//  image = QImage(QSize(xCount),QSize(yCount));
  image = QImage(window.size(),QImage::Format_ARGB32_Premultiplied);
  for(int x=0; x<=xCount; x++)
  {
    for (int y=0; y<=yCount; y++)
    {
     // qDebug() << x << y << osm_x+x << osm_y+y;
      QTransform t;
      t = t.translate(x*256,y*256);
     // qDebug() << window << point << t.map(point) << -osm_x_delta*256 << -osm_y_delta*256;
     // qDebug() << QString("punkt:%1  t: %2").arg(point).arg(t.map(point));
      getImage(osm_zoom,osm_x+x,osm_y+y,t.map(point));
    }
  }
  emit newImageReady(image);
}

void COsmTilesHash::getImage(int osm_zoom, int osm_x, int osm_y, QPoint point)
{
  // *  Tiles are 256 × 256 pixel PNG files
  // * Each zoom level is a directory, each column is a subdirectory, and each tile in that column is a file
  // * Filename(url) format is /zoom/x/y.png
  QString osmUrlPart = QString("/%1/%2/%3.png").arg(osm_zoom).arg(osm_x).arg(osm_y);
  QString osmFilePath = QDir::tempPath() + "/qlandkarteqt/cache/" + osmUrlPart;

 // qDebug() << osmUrlPart;
  bool needHttpAction = true;
  if (tiles.contains(osmUrlPart))
  {
    //qDebug() << "cached";
    QPainter p(&image);
    p.drawImage(point,tiles.value(osmUrlPart));
    p.drawRect(QRect(point,QSize(255,255)));
    p.drawText(point + QPoint(10,10), "cached " + osmUrlPart);
    needHttpAction = false;
  }
  else if (QFileInfo(osmFilePath).exists())
  {
    QFile f(osmFilePath);
    if (f.open(QIODevice::ReadOnly))
    {
      QImage img1;
      img1.loadFromData(f.readAll());

      if(img1.format() != QImage::Format_Invalid)
      {
        QPainter p(&image);
        p.drawImage(point,img1);
        tiles.insert(osmUrlPart,img1);
        if (QFileInfo(osmFilePath).lastModified().daysTo(QDateTime::currentDateTime()) < 8)
        {
          needHttpAction = false;
        }
      }
    }
  }
  //needHttpAction = true;
  if (needHttpAction && !osmRunningHash.contains(osmUrlPart))
  {
    QPainter p(&image);
    getid = tilesConnection->get(osmUrlPart);
    osmRunningHash.insert(osmUrlPart,getid);
    p.drawText(point + QPoint(20,128), tr("Image is loading: %1").arg(osmUrlPart));
    p.drawText(point + QPoint(20,148), tr("%1 of %2 stored.").arg(tiles.count()).arg(getid));
    startPointHash.insert(getid, point);
    osmUrlPartHash.insert(getid, osmUrlPart);

  }
}


void COsmTilesHash::slotRequestFinished(int id, bool error)
{
  qDebug() << osmUrlPartHash.value(id) << id << error ;
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
  if (f.open(QIODevice::WriteOnly))
  {
    img1.save ( &f);
  }

  tiles.insert(osmUrlPart,img1);
  if (osmUrlPart.startsWith(QString("/%1/").arg(osm_zoom)) && startPointHash.contains(id))
  {
    QPainter p(&image);
    //qDebug() << "startPointHash.value(id)" << startPointHash.value(id);
    p.drawImage(startPointHash.value(id),img1);
    p.drawRect(QRect(startPointHash.value(id),QSize(255,255)));
    p.drawText(startPointHash.value(id) + QPoint(10,10), QString::number(id) + osmUrlPartHash.value(id));
    osmUrlPartHash.remove(id);
    startPointHash.remove(id);
    osmRunningHash.remove(osmUrlPart);
    emit newImageReady(image);
  }
  return;
}

int COsmTilesHash::long2tile(double lon, int z)
{
  return (int)(qRound(256*(lon + 180.0) / 360.0 * pow(2.0, z)));
//  return (lon + 180.0) / 360.0 * pow(2.0, z);
}

int COsmTilesHash::lat2tile(double lat, int z)
{
  return (int)(qRound(256*(1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
//  return (1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z);
}

double COsmTilesHash::tile2long(int x, int z)
{
  return x / pow(2.0, z) * 360.0 - 180.;
}

double COsmTilesHash::tile2lat(int y, int z)
{
  double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
  return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}


