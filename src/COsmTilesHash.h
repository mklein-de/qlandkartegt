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

#ifndef COSMTILESHASH_H_
#define COSMTILESHASH_H_
#include <QObject>
#include <QString>
#include <QRect>
#include <QPainter>
#include <QPixmap>
#include <QHash>
#include <QQueue>
#include <QUrl>
#include <QNetworkRequest>

class QNetworkAccessManager;
class QNetworkReply;
class CMapOSM;
class QNetworkDiskCache;

class COsmTilesHash: public QObject
{
    Q_OBJECT
        public:
        COsmTilesHash(QString tileUrl, QObject * parent);
        virtual ~COsmTilesHash();
        void startNewDrawing( double lon, double lat, int osm_zoom, const QRect& window);

    signals:
        void newImageReady(const QPixmap& image, bool lastTileLoaded);
    private:
        QUrl m_tileUrl;
        QString m_tilePath;
        QNetworkDiskCache * diskCache;

        QQueue<QPair<QNetworkRequest, QPoint > > m_queuedRequests;
        QHash<QString, QPoint> m_activeRequests;
        QHash<QString, QPixmap>  m_tileHash;
        int long2tile(double lon, int zoom);
        int lat2tile(double lat, int zoom);
        double tile2long(int x, int zoom);
        double tile2lat(int y, int zoom);
        void getImage(int osm_zoom, int osm_x, int osm_y, QPoint startPoint);
        QPixmap pixmap;
        QNetworkAccessManager *m_networkAccessManager;

        void dequeue();
    private slots:
        void slotRequestFinished(QNetworkReply*);
};
#endif                           /* COSMTILESHASH_H_ */
