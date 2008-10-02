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

#include "IMap.h"

#include <QVector>
#include <QRgb>

class GDALDataset;
class QImage;
class CStatusDEM;

class CMapDEM : public IMap
{
    Q_OBJECT;
    public:
        CMapDEM(const QString& filename, CCanvas * parent, const QString& datum, const QString& gridfile);
        virtual ~CMapDEM();

        /// draw map
        void draw(QPainter& p);

        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);
        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void select(const QRect& rect);
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);
        float getElevation(float lon, float lat);

    private:
        void shading(QImage& img, qint16 * data);
        void contour(QImage& img, qint16 * data);

        /// instance of GDAL dataset
        GDALDataset * dataset;
        /// width of GeoTiff tiles / blocks
        qint32 tileWidth;
        /// height of GeoTiff tiles / blocks
        qint32 tileHeight;

        /// width in number of px
        quint32 xsize_px;
        /// height in number of px
        quint32 ysize_px;

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

        QVector<QRgb> graytable1;
        QVector<QRgb> graytable2;

        CStatusDEM * status;

        struct weight_t
        {
            weight_t(): c1(0), c2(0), c3(0), c4(0) {}
            float c1;
            float c2;
            float c3;
            float c4;
        };

        weight_t * weights;

};
#endif                           //CMAPDEM_H
