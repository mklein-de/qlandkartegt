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
#ifndef CMAPTDB_H
#define CMAPTDB_H

#include "IMap.h"
#include <QMap>

class CMapGarminTile;

class CMapTDB : public IMap
{
    Q_OBJECT;
    public:
        CMapTDB(const QString& key, const QString& filename, CCanvas * parent);
        virtual ~CMapTDB();

        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);
        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void zoom(qint32& level);

        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);

        const QString& getName(){return name;}

    private:
        void readTDB(const QString& filename);
        bool processPrimaryMapData();

#pragma pack(1)
        struct tdb_hdr_t
        {
            quint8  type;
            quint16 size;
        };

        struct tdb_product_t : public tdb_hdr_t
        {
            quint32 id;
            quint16 version;
            char *  name[];
        };

        struct tdb_map_t : public tdb_hdr_t
        {
            quint32 id;
            quint32 country;
            qint32  north;
            qint32  east;
            qint32  south;
            qint32  west;
            char    name[];
        };

        struct tdb_map_size_t
        {
            quint16 dummy;
            quint16 count;
            quint32 sizes[1];
        };

        struct tdb_copyright_t
        {
            quint8  type;
            quint16 count;
            quint8  flag;
            char    str[];
        };

        struct tdb_copyrights_t : public tdb_hdr_t
        {
            tdb_copyright_t entry;
        };
#pragma pack(0)

        struct tile_t
        {
            quint32 id;
            QString key;
            QString name;
//             std::string cname;
            QString file;
            double north;
            double east;
            double south;
            double west;
            QRectF area;
//             QVector<XY> definitionArea;
            CMapGarminTile * img;
            quint32 memSize;
        };
        /// tdb filename
        QString filename;
        /// copyright string
        QString copyright;
        /// map collection name
        QString name;
        /// basemap filename
        QString basemap;
        /// north boundary of basemap [°]
        double north;
        /// east boundary of basemap [°]
        double east;
        /// south boundary of basemap [°]
        double south;
        /// west boundary of basemap [°]
        double west;
        /// the area in [m] covered by the basemap
        QRect area;
        /// set true for encrypted maps
        bool encrypted;
        /// the unlock key
        QString key;
        /// the basemap tile;
        CMapGarminTile * img;
        /// high detail map tiles
        QMap<QString,tile_t> tiles;
};

#endif //CMAPTDB_H

