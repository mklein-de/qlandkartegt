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
#ifndef CSEARCHDB_H
#define CSEARCHDB_H

#include "IDB.h"

#include <QList>

class QHttp;

/// search database
class CSearchDB : public IDB
{
    Q_OBJECT
    public:
        virtual ~CSearchDB();

        struct result
        {
            qreal   lon;
            qreal   lat;
            QString label;
        };

        void search(const QString& str);

        /// get iterator access to track point list
        QList<result>::iterator begin(){return results.begin();}
        /// get iterator access to track point list
        QList<result>::iterator end(){return results.end();}

    signals:
        void sigStatus(const QString& msg);

    private slots:
        void slotSetupLink();
        void slotRequestStarted(int );
        void slotRequestFinished(int , bool error);

    private:
        friend class CMainWindow;

        CSearchDB(QToolBox * tb, QObject * parent);
        static CSearchDB * m_self;

        QHttp * google;
        QList<result> results;
};

#endif //CSEARCHDB_H

