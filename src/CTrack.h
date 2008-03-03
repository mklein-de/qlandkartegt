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
#include <QPolygon>
#include "CWpt.h"

class CTrack : public QObject
{
    Q_OBJECT
    public:
        CTrack(QObject * parent);
        virtual ~CTrack();

        enum type_e {eEnd,eBase,eTrkPts};

        struct pt_t {

            enum flag_e
            {
                 eSelected  = 1       ///< selected by track info view
                ,eCursor    = 2      ///< selected by cursor
                ,eDeleted   = 4      ///< mark point as deleted
                ,eFocus     = 8      ///< mark current point of user focus

            };

            pt_t() : idx(-1), lon(WPT_NOFLOAT), lat(WPT_NOFLOAT), ele(WPT_NOFLOAT), timestamp(0),
                     speed(WPT_NOFLOAT), delta(WPT_NOFLOAT), azimuth(WPT_NOFLOAT), distance(WPT_NOFLOAT), flags(0){}
            /// index counter for easy QVector access
            qint32  idx;
            /// longitude [°]
            float   lon;
            /// latitude [°]
            float   lat;
            /// elevation [m]
            float   ele;
            /// timestamp for track creation
            quint32 timestamp;

            /// secondary data: the speed between this and the previous point
            float speed;
            /// secondary data: the distance between this and the previous point
            float delta;
            /// secondary data: the azimuth to the next point
            double azimuth;
            /// secondary data: the total distance of all visible points up to this point
            float distance;
            /// display flags
            quint32 flags;
            /// the current location in pixel
            QPoint px;
        };

        /// set color by id
        void setColor(unsigned i);
        /// get QT color
        const QColor& getColor(){return color;}
        const unsigned getColorIdx(){return colorIdx;}
        /// set track name
        void setName(const QString& n){name = n;}
        /// get track name
        const QString& getName(){return name;}
        /// get unique track key
        const QString& key();

        /// set the highlight flag
        void setHighlight(bool yes){highlight = yes;}
        /// get the value of the highlight flag
        bool isHighlighted(){return highlight;}

        /// append point to track
        CTrack& operator<<(pt_t& pt);
        /// rebuild secondary track data from primary
        void rebuild(bool reindex);
        /// get list of track points
        QVector<pt_t>& getTrackPoints(){return track;};
        /// get polyline representation of track
        QPolygon& getPolyline(){return polyline;}
        /// get the total distance of the track in [m]
        double getTotalDistance(){return totalDistance;}
        /// get the total time covered by the track in seconds
        int getTotalTime(){return totalTime;}
        /// select tarckpoint by index
        void setPointOfFocus(int idx);

        static const QColor colors[];

    signals:
        void sigChanged();

    private:
        friend QDataStream& operator >>(QDataStream& s, CTrack& track);
        friend QDataStream& operator <<(QDataStream& s, CTrack& track);
        void genKey();

        static QDir path;

        /// unique key to address tarck
        QString _key_;
        /// creation timestamp
        quint32 timestamp;
        /// track name
        QString name;
        /// a comment string
        QString comment;
        /// the track line color
        QColor  color;
        /// the track line color by index
        unsigned colorIdx;
        /// the track points
        QVector<pt_t> track;

        /// set true to draw track highlighted
        bool highlight;

        /// total time covered by all track points
        quint32 totalTime;
        /// total distance of track [m]
        double  totalDistance;

        /// the Qt polyline for faster processing
        QPolygon polyline;

};

QDataStream& operator >>(QDataStream& s, CTrack& track);
QDataStream& operator <<(QDataStream& s, CTrack& track);

void operator >>(QFile& f, CTrack& track);
void operator <<(QFile& f, CTrack& track);


#endif //CTRACK_H

