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

#ifndef CTRACK_H
#define CTRACK_H

#include <QObject>
#include <QVector>
#include <QColor>
#include "CWpt.h"

class CTrack : public QObject
{
    Q_OBJECT
    public:
        CTrack(QObject * parent);
        virtual ~CTrack();

        struct pt_t {

            enum flag_e
            {
                 eSelected  = 1       ///< selected by track info view
                ,eCursor    = 2      ///< selected by cursor
                ,eDeleted   = 4      ///< mark point as deleted
                ,eFocus     = 8      ///< mark current point of user focus

            };

            pt_t() : idx(-1), lon(WPT_NOFLOAT), lat(WPT_NOFLOAT), ele(WPT_NOFLOAT), time(0),
                     speed(WPT_NOFLOAT), delta(WPT_NOFLOAT), azimuth(WPT_NOFLOAT), distance(WPT_NOFLOAT), flags(0){}
            qint32  idx;
            float   lon;
            float   lat;
            float   ele;
            quint32 time;

            /// secondary data: the speed between this and the previous point
            float speed;
            /// secondary data: the distance between this and the previous point
            float delta;
            /// secondary data: the azimuth to the next point
            float azimuth;
            /// secondary data: the total distance of all visible points up to this point
            float distance;
            /// display flags
            quint32 flags;

        };

        /// set color by id
        void setColor(unsigned i);
        /// get QT color
        const QColor& getColor(){return color;}
        /// set track name
        void setName(const QString& n){name = n;}
        /// get track name
        const QString& getName(){return name;}
        /// get unique track key
        const QString& key();
        /// append point to track
        CTrack& operator<<(pt_t& pt);
        /// rebuild secondary track data from primary
        void rebuild(bool reindex);

    signals:
        void sigChanged();

    private:
        void genKey();
        static const QColor colors[];

        QString _key_;
        quint32 timestamp;
        QString name;
        QString comment;
        QColor  color;
        QVector<pt_t> track;

        quint32 totalTime;
        double  totalDistance;

};

#endif //CTRACK_H

