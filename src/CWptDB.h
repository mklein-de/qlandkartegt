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

class CWptToolWidget;

/// waypoint database
class CWptDB : public IDB
{
    Q_OBJECT
    public:
        virtual ~CWptDB();

        static CWptDB& self(){return *m_self;}

    private:
        friend class CMainWindow;

        CWptDB(QToolBox * tb, QObject * parent);
        static CWptDB * m_self;

};

#endif //CWPTDB_H

