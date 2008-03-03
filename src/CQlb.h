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
#ifndef CQLB_H
#define CQLB_H

#include <QObject>
#include <QByteArray>

class CWpt;
class CTrack;

/// qlandkarte binary to store privat geo data
/**
    The file will store data like waypoints, tracks, map selections. These elements will
    be collected in a dedicated byte arra, e.g. all waypoints are serialized in wpts and so on.
    These byte arrays a stored like:

    qint32 eWpt, QByteArray wpts
    ...
    qint32 eEnd

*/
class CQlb : public QObject
{
    Q_OBJECT
    public:
        CQlb(QObject * parent);
        virtual ~CQlb();

        enum type_e {eEnd, eWpt, eTrack};

        /// collect wapoint data
        /**
            This will serialize the waypoint object to wpts
        */
        CQlb& operator <<(CWpt& wpt);

        CQlb& operator <<(CTrack& trk);

        /// get access to stored waypoint data
        QByteArray& waypoints(){return wpts;}
        /// get access to stored track data
        QByteArray& tracks(){return trks;}
        /// write collected data to file
        void save(const QString& filename);
        /// read file and store elements in their designated byte arrays
        void load(const QString& filename);

    private:
        /// byte array to hold all
        QByteArray wpts;
        /// byte array to hold all
        QByteArray trks;

};

#endif //CQLB_H

