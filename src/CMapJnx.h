/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CMAPJNX_H
#define CMAPJNX_H

#include "IMap.h"

class CMapJnx : public IMap
{
    Q_OBJECT;
    public:
        CMapJnx(const QString& key, const QString& filename, CCanvas * parent);
        virtual ~CMapJnx();

        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);
        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void zoom(qint32& level);
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);

        QString getName(){return name;}

        void draw(QPainter& p);

        void getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale);

        QString getMapInfo(){return info;}

        void config();

    private:
        void draw();
        qint32 zlevel2idx(quint32);

        QString name;

        double lon1;
        double lat1;
        double lon2;
        double lat2;

#pragma pack(1)
        struct hdr_t
        {
            quint32 version;            // byte 00000000..00000003
            quint32 devid;              // byte 00000004..00000007
            qint32  top;              // byte 00000010..00000013
            qint32  right;              // byte 00000014..00000017
            qint32  bottom;              // byte 00000008..0000000B
            qint32  left;              // byte 0000000C..0000000F
            quint32 details;            // byte 00000018..0000001B
            quint64 expire;             // byte 0000001C..00000023
            quint32 crc;                // byte 00000024..00000027
            quint32 signature;          // byte 00000028..0000002B
            quint32 signature_offset;   // byte 0000002C..0000002F
            quint32 zorder;             // byte 00000030--00000033
        };

#ifdef WIN32
#pragma pack()
#else
#pragma pack(0)
#endif

        struct tile_t
        {
            QRectF  area;
            quint16 width;
            quint16 height;
            quint32 size;
            quint32 offset;
        };

        struct level_t
        {
            quint32 nTiles;
            quint32 offset;
            quint32 scale;
            QString copyright1;

            quint32 level;
            QString name1;
            QString name2;
            QString copyright2;

            QVector<tile_t> tiles;
        };

        /// scale entry
        struct scale_t
        {
            /// scale factor
            double  qlgtScale;
            quint32 jnxScale;
        };

        static scale_t scales[];

        QVector<level_t> levels;

        double x;
        double y;

        double xscale;
        double yscale;

        double zoomFactor;


        QRectF viewport;

        QString info;
};

#endif //CMAPJNX_H

