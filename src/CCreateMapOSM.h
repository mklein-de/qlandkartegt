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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

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
class CCreateMapOSM : public QWidget, private Ui::ICreateMapOSM
{
    Q_OBJECT
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


        struct zoomlevel_t
        {
            zoomlevel_t() : dataset(0), band(0), zoom(-1){}
            zoomlevel_t(int zoom) : dataset(0), band(0), zoom(zoom){}
            ~zoomlevel_t();

            GDALDataset * dataset;
            GDALRasterBand * band;
            int zoom;
        };

        struct tile_t
        {
            tile_t() : zoomlevel(0), zoom(-1), x(-1), y(-1), done(false){};
            zoomlevel_t * zoomlevel;
            int zoom;
            int x;
            int y;
            QUrl url;
            bool done;
        };

        QVector<zoomlevel_t> zoomlevels;

        QVector<tile_t> tiles;

        int maxTiles;

        QPointer<QProgressDialog> progress;

};

#endif //CCREATEMAPOSM_H

