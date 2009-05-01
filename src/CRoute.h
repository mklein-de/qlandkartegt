/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CROUTE_H
#define CROUTE_H

#include <QObject>
#include <QPixmap>
#include <QList>
#include <QPolygon>
#include <projects.h>

class CRoute : public QObject
{
    Q_OBJECT;
    public:
        CRoute(QObject * parent);
        virtual ~CRoute();

        /// set route name
        void setName(const QString& n){name = n;}
        /// get route name
        const QString& getName(){return name;}
        /// get unique track key
        const QString& key();

        double getDistance(){return dist;}

        const QPixmap& getIcon(){return icon;}

        /// set the highlight flag
        void setHighlight(bool yes){highlight = yes;}
        /// get the value of the highlight flag
        bool isHighlighted(){return highlight;}
        /// add a new position point
        /**
            @param lon the longitude in degree
            @param lat the latitude in degree
        */
        void addPosition(const double lon, const double lat);

        QPolygon& getPolyline(){return polyline;}

        QList<XY>& getRoutePoints(){return routeDegree;}

        QRectF getBoundingRectF();

    signals:
        void sigChanged();

    private:
        friend class CRouteDB;
        void genKey();
        void calcDistance();

        /// unique key to address tarck
        QString _key_;
        /// creation timestamp
        quint32 timestamp;

        /// route name (used as lookup key, too)
        QString name;
        /// the route as position points
        QList<XY> routeDegree;
        /// the actual route distance
        double dist;

        QString iconname;

        /// the icon used to draw a route
        QPixmap icon;

        bool highlight;

        /// the Qt polyline for faster processing
        QPolygon polyline;

        bool firstTime;

};

#endif //CROUTE_H

