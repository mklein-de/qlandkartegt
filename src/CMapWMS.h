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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/
#ifndef CMAPWMS_H
#define CMAPWMS_H

#include "IMap.h"
#include <QVector>
#include <QRgb>

class GDALDataset;

class CMapWMS  : public IMap
{
    Q_OBJECT;
    public:
        CMapWMS(const QString& key, const QString& fn, CCanvas * parent);
        virtual ~CMapWMS();

        void draw(QPainter& p);
        void draw();
        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);
        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void zoom(qint32& level);
        void select(const QRect& rect);
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);
        void getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale);

    private:
        /// instance of GDAL dataset
        GDALDataset * dataset;

        double x;
        double y;

        double zoomFactor;

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

        /// the longitude of the top left reference point [rad]
        double lon1;
        /// the latitude of the top left reference point [rad]
        double lat1;
        /// the longitude of the bottom right reference point [rad]
        double lon2;
        /// the latitude of the bottom right reference point [rad]
        double lat2;

};
#endif                           //CMAPRASTER_H
