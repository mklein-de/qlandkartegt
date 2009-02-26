//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2009 Marc Feld
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


#ifndef COSMTILESHASH_H_
#define COSMTILESHASH_H_
#include <QObject>
#include <QString>
#include <QRect>
#include <QPainter>
#include <QImage>
#include <QHash>

class QHttp;
class CMapOSM;
class COsmTilesHash: public QObject
{
  Q_OBJECT
public:
  COsmTilesHash(CMapOSM *cmapOSM);
  virtual ~COsmTilesHash();
  void startNewDrawing( double lon, double lat, int x, int y, int osm_zoom, const QRect& window);
signals:
  void newImageReady(QImage image);
private:
  int x;
  int y;
  int osm_zoom;
  QRect window;
  QHash<int, QPoint> startPointHash;
  QHash<int, QString> osmUrlPartHash;
  int long2tile(double lon, int zoom);
  int lat2tile(double lat, int zoom);
  double tile2long(int x, int zoom);
  double tile2lat(int y, int zoom);
  void getImage(int osm_zoom, int osm_x, int osm_y, QPoint startPoint);
  QImage image;
  QHttp *tilesConnection;
  CMapOSM *cmapOSM;
  QString osmTileBaseUrl;
  bool requestInProgress;
  QHash<QString,QImage> tiles;
  int getid;
private slots:
   // void slotCreate();
    void slotRequestFinished(int , bool error);
   // void slotSelectPath();
};

#endif /* COSMTILESHASH_H_ */
