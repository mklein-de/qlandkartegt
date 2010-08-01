/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de
    Copyright (C) 2010 Joerg Wunsch <j@uriah.heep.sax.de>

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
#ifndef IDB_H
#define IDB_H

#include <QObject>

class QTabWidget;
class QWidget;
class QPainter;
class QRect;
class CGpx;
class CQlb;
class QString;

/// base class for all database objects
class IDB : public QObject
{
    Q_OBJECT;
    public:
        IDB(QTabWidget * tb, QObject * parent);
        virtual ~IDB();

        /// move database views into focus
        /**
            If this is called the database should try to make all it's
            toolviews visible to the user.
        */
        virtual void gainFocus();

        /// parse a GPX timestamp, including timezone calculations
        virtual bool parseTimestamp(const QString &time, quint32 &tstamp);
        virtual bool parseTimestamp(const QString &time, quint32 &tstamp,
                                    quint32 &tstamp_msec);

        /// load database data from gpx
        virtual void loadGPX(CGpx& gpx) = 0;
        /// save database data to gpx
        virtual void saveGPX(CGpx& gpx, const QStringList& keys) = 0;

        /// load database data from QLandkarte binary
        virtual void loadQLB(CQlb& qlb) = 0;
        /// save database data to QLandkarte binary
        virtual void saveQLB(CQlb& qlb) = 0;

        virtual void upload(const QStringList& keys) = 0;
        virtual void download() = 0;

        virtual void clear() = 0;

        virtual int count(){return -1;}

        virtual void draw(QPainter& p, const QRect& rect, bool& needsRedraw){}

    signals:
        void sigChanged();
        void sigModified();

    protected:
        QTabWidget *  tabbar;
        QWidget *  toolview;
};
#endif                           //IDB_H
