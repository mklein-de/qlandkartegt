/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CCREATEMAPOSM_H
#define CCREATEMAPOSM_H

#include <QWidget>
#include <QUrl>
#include <QPointer>
#include <gdal_priv.h>
#include "ui_ICreateMapOSM.h"

class QHttp;
class QProgressDialog;
class QSettings;

/// download tiles from OSM and stitch them together
/**
    Tile are loaded from tah.openstreetmap.org. The user defines the
    desired area via geo. coordinates of the top left and bottom right
    corner. The tiles will be stitched together to one singel GeoTiff
    file with a unified 8bit palette. Altogether three map files of
    different resolution are created. To speak in OSM terms these area
    from the zoom levels 17, 14 and 11.
*/
class CCreateMapOSM : public QWidget, private Ui::ICreateMapOSM
{
    Q_OBJECT;
    public:
        CCreateMapOSM(QWidget * parent);
        virtual ~CCreateMapOSM();

    private slots:
        void slotCreate();
        void slotRequestFinished(int , bool error);
        void slotSelectPath();

    private:
        void getNextTile();
        void addZoomLevel(int level, int zoom, double lon1, double lat1, double lon2, double lat2, QSettings& mapdef);
        void finishJob();

        QHttp * link;

        /// zoomlevel definition
        struct zoomlevel_t
        {
            zoomlevel_t() : dataset(0), band(0){}
            ~zoomlevel_t();

            /// the GeoTiff's dataset
            GDALDataset * dataset;
            /// the GeoTiff's rasterband
            GDALRasterBand * band;
        };

        /// tile definition
        struct tile_t
        {
            tile_t() : zoomlevel(0), zoom(-1), x(-1), y(-1), done(false){};
            /// pointer to zoomlevel definition the tile is used in
            zoomlevel_t * zoomlevel;
            /// OSM zoom level
            int zoom;
            /// GeoTiff x offset
            int x;
            /// GeoTiff y offset
            int y;
            /// URL for download
            QUrl url;
            /// will be true if tile is processed
            bool done;
        };

        /// list of zoomlevel definitions used to create the map
        QVector<zoomlevel_t> zoomlevels;
        /// list of tile definitions to download
        QVector<tile_t> tiles;
        /// complete number of tiles to download
        int maxTiles;
        /// show progress of all download transactions
        QPointer<QProgressDialog> progress;

};
#endif                           //CCREATEMAPOSM_H
