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

#ifndef CROUTEDB_H
#define CROUTEDB_H

#include "IDB.h"

class CRouteDB : public IDB
{
    Q_OBJECT;
    public:
        virtual ~CRouteDB();

        static CRouteDB& self(){return *m_self;}

        /// load database data from gpx
        void loadGPX(CGpx& gpx);
        /// save database data to gpx
        void saveGPX(CGpx& gpx);

        /// load database data from QLandkarte binary
        void loadQLB(CQlb& qlb);
        /// save database data to QLandkarte binary
        void saveQLB(CQlb& qlb);

        void upload();
        void download();
        void clear();


    private:
        friend class CMainWindow;

        CRouteDB(QTabWidget * tb, QObject * parent);

        static CRouteDB * m_self;

};

#endif //CROUTEDB_H

