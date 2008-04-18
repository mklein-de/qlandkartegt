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

#ifndef CDIARYDB_H
#define CDIARYDB_H

#include "IDB.h"
#include "CDiary.h"

#include <QPointer>

class CDiaryEditWidget;

class CDiaryDB : public IDB
{
    Q_OBJECT
    public:
        virtual ~CDiaryDB();

        static CDiaryDB& self(){return *m_self;}

        void openEditWidget();
        const QString getDiary();

        void loadGPX(CGpx& gpx){};
        void saveGPX(CGpx& gpx){};

        void loadQLB(CQlb& qlb);
        void saveQLB(CQlb& qlb);

        void upload(){};
        void download(){};

        void clear();

    private:
        friend class CMainWindow;
        friend class CDiaryEditWidget;

        CDiaryDB(QTabWidget * tb, QObject * parent);

        static CDiaryDB * m_self;

        QPointer<CDiaryEditWidget> editWidget;

        CDiary diary;
};

#endif //CDIARYDB_H

