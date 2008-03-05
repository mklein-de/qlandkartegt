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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#ifndef CMAPDEM_H
#define CMAPDEM_H

#include <QObject>
#include <QVector>
#include <QRgb>
#include <projects.h>

#include "IMap.h"

class GDALDataset;
class QPainter;
class QSize;
class QImage;

/// data object for digital elevation models
/**
    Get the elevation for a given point or apply elevation shading to the canvas.
    The data is assumed to be 16bit per point.
*/
class CMapDEM : public QObject
{
    Q_OBJECT;
    public:
        CMapDEM(const QString& filename, QObject * parent);
        virtual ~CMapDEM();

        /// read elevation from file
        /**
            @param lon the longitude in [rad]
            @param lat the latitude in [rad]
        */
        float getElevation(float& lon, float& lat);

        void draw(QPainter& p, const XY& p1, const XY& p2, const float my_xscale, const float my_yscale, IMap::overlay_e overlay);

    private:
        void shading(QImage& img, qint16 * data);
        void contour(QImage& img, qint16 * data);

        QString filename;
        /// instance of GDAL dataset
        GDALDataset * dataset;
        /// width in number of px
        quint32 xsize_px;
        /// height in number of px
        quint32 ysize_px;
        /// configuration string for projection
        QString strProj;
        /// projection context of DEM data
        PJ * pjsrc;
        /// projection context for WGS84
        PJ * pjtar;

        /// scale [px/m]
        double xscale;
        /// scale [px/m]
        double yscale;
        /// reference point [m] (left hand side of map)
        double xref1;
        /// reference point [m] (top of map)
        double yref1;
        /// reference point [m] (right hand side of map)
        double xref2;
        /// reference point [m] (bottom of map)
        double yref2;

        /// width of GeoTiff tiles / blocks
        qint32 tileWidth;
        /// height of GeoTiff tiles / blocks
        qint32 tileHeight;

        QVector<QRgb> graytable1;
        QVector<QRgb> graytable2;

};
#endif                           //CMAPDEM_H
