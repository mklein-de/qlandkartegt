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

#ifndef CLIVELOGDB_H
#define CLIVELOGDB_H

#include "IDB.h"
#include "CLiveLog.h"

#include <QPolygon>

class QPainter;
class QFile;

class CLiveLogDB : public IDB
{
    Q_OBJECT;
    public:
        virtual ~CLiveLogDB();

        struct simplelog_t
        {
            simplelog_t() : timestamp(0xFFFFFFFF), lon(WPT_NOFLOAT), lat(WPT_NOFLOAT), ele(WPT_NOFLOAT) {}
            quint32 timestamp;
            float lon;
            float lat;
            float ele;
        };

        static CLiveLogDB& self(){return *m_self;}

        void draw(QPainter& p, const QRect& rect, bool& needsRedraw);

        void loadGPX(CGpx& /*gpx*/){};
        void saveGPX(CGpx& /*gpx*/){};

        void loadQLB(CQlb& /*qlb*/){};
        void saveQLB(CQlb& /*qlb*/){};

        void upload(){};
        void download(){};

        void clear();

        void start(bool yes);
        bool logging();

        void setLockToCenter(bool on){m_lockToCenter = on;}
        bool lockToCenter(){return m_lockToCenter;}

        void addWpt();

    private slots:
        void slotLiveLog(const CLiveLog& log);
        void slotMapChanged();
    private:

        friend class CMainWindow;
        CLiveLogDB(QTabWidget * tb, QObject * parent);
        void saveBackupLog();

        static CLiveLogDB * m_self;
        CLiveLog m_log;

        QVector<simplelog_t> track;
        QPolygon polyline;

        bool m_lockToCenter;

        QFile * backup;
};
#endif                           //CLIVELOGDB_H
