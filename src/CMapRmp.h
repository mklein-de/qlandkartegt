/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CMAPRMP_H
#define CMAPRMP_H

#include "IMap.h"

#include <QtCore>

class CMapRmp : public IMap
{
    Q_OBJECT;
    public:
        CMapRmp(const QString& key, const QString& fn, CCanvas * parent);
        virtual ~CMapRmp();

        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);

        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p0);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void zoom(qint32& level);
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);
        void getArea_n_Scaling(projXY& p1, projXY& p2, float& my_xscale, float& my_yscale);

        QString getName(){return name;}

        void draw(QPainter& p);


    private:
        QString name;


        struct dir_entry_t
        {
            dir_entry_t() : offset(0), length(0), name(10,0), extension(8,0){}


            quint32 offset;
            quint32 length;
            QString name;
            QString extension;

        };

        struct tlm_t : public dir_entry_t
        {

            quint32 tileCount;
            quint16 tileXSize;
            quint16 tileYSize;

            double tileHeigth;
            double tileWidth;
            double tileLeft;
            double tileTop;
            double tileRight;
            double tileBottom;

            tlm_t& operator=(dir_entry_t& entry)
            {
                offset      = entry.offset;
                length      = entry.length;
                name        = entry.name;
                extension   = entry.extension;

                return *this;
            }
        };

        struct a00_t : public dir_entry_t
        {
            a00_t& operator=(dir_entry_t& entry)
            {
                offset      = entry.offset;
                length      = entry.length;
                name        = entry.name;
                extension   = entry.extension;

                return *this;
            }
        };

        struct level_t
        {
            QString name;
            tlm_t tlm;
            a00_t a00;
        };

        QList<dir_entry_t> directory;

        QMap<QString,level_t> levels;

        /// scale entry
        struct scale_t
        {
            /// scale factor
            double  qlgtScale;
            quint32 jnxScale;
        };

        static scale_t scales[];


        /// reference point [m] (left hand side of map)
        double xref1;
        /// reference point [m] (top of map)
        double yref1;
        /// reference point [m] (right hand side of map)
        double xref2;
        /// reference point [m] (bottom of map)
        double yref2;

        double x;
        double y;

        double xscale;
        double yscale;

        double zoomFactor;

};

#endif //CMAPRMP_H

