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

#ifndef CMAPFILE_H
#define CMAPFILE_H

#include <QObject>
#include <QRgb>
#include <QVector>
#include <projects.h>

class CMapLevel;
class GDALDataset;

/// data object for easy access to GeoTiff information
/**
    This will create a dataset and read basic information
    from the GeoTiff header.
*/
class CMapFile : public QObject
{
    Q_OBJECT
    public:
        CMapFile(const QString& filename, CMapLevel * parent);
        virtual ~CMapFile();

        /// source file name
        QString filename;

        /// instance of GDAL dataset
        GDALDataset * dataset;
        /// width in number of px
        quint32 xsize_px;
        /// height in number of px
        quint32 ysize_px;
        /// configuration string for projection
        QString strProj;
        /// projection context
        PJ * pj;

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

        /// QT representation of the GeoTiff's color table
        QVector<QRgb> colortable;

        /// width of GeoTiff tiles / blocks
        qint32 tileWidth;
        /// height of GeoTiff tiles / blocks
        qint32 tileHeight;

};

#endif //CMAPFILE_H

