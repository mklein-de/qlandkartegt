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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/
#ifndef CWPTDB_H
#define CWPTDB_H

#include "IDB.h"

#include <QString>
#include <QMap>

class CWptToolWidget;
class CWpt;

/// waypoint database
class CWptDB : public IDB
{
    Q_OBJECT;
    public:
        virtual ~CWptDB();

        static CWptDB& self(){return *m_self;}

        void draw(QPainter& p, const QRect& rect, bool& needsRedraw);

        /// get iterator access to track point list
        QMap<QString,CWpt*> ::iterator begin(){return wpts.begin();}
        /// get iterator access to track point list
        QMap<QString,CWpt*> ::iterator end(){return wpts.end();}

        const QMap<QString,CWpt*>& getWpts(){return wpts;}

        /// create a new waypoint
        /**
            @param lon longitude in [rad]
            @param lat latitude in [rad]
            @param ele elevation in [m]

            @return A temporary pointer to the waypoint object or 0 if the waypoint dialog was canceled.
        */
        CWpt * newWpt(float lon, float lat, float ele);

        /// get pointer access to waypoint via it's key
        CWpt * getWptByKey(const QString& key);

        /// delete several waypoints by their keys
        void delWpt(const QStringList& keys, bool saveSticky = true);
        /// delete a waipoint by it's key
        void delWpt(const QString& key, bool silent = false, bool saveSticky = true);

        void loadGPX(CGpx& gpx);
        void saveGPX(CGpx& gpx);
        void loadQLB(CQlb& qlb);
        void saveQLB(CQlb& qlb);

        void upload();
        void download();

        void clear();

        void selWptByKey(const QString& key);

        int count(){return wpts.count();}

    private:
        friend class CMainWindow;
        friend class CDlgEditWpt;
        friend class CMouseMoveWpt;

        CWptDB(QTabWidget * tb, QObject * parent);
        void addWpt(CWpt * wpt);
        static CWptDB * m_self;

        QMap<QString,CWpt*> wpts;

};
#endif                           //CWPTDB_H
