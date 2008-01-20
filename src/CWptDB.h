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
    Q_OBJECT
    public:
        virtual ~CWptDB();

        static CWptDB& self(){return *m_self;}

        /// get iterator access to track point list
        QMap<QString,CWpt*> ::iterator begin(){return wpts.begin();}
        /// get iterator access to track point list
        QMap<QString,CWpt*> ::iterator end(){return wpts.end();}

        /// create a new waypoint
        void newWpt(double lon, double lat);

    private:
        friend class CMainWindow;

        CWptDB(QToolBox * tb, QObject * parent);
        static CWptDB * m_self;

        QMap<QString,CWpt*> wpts;

};

#endif //CWPTDB_H

