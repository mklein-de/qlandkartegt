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

#ifndef COVERLAYDB_H
#define COVERLAYDB_H

#include "IDB.h"

#include <QMap>
#include <projects.h>

class QPoint;
class QRect;
class IOverlay;
class QPainter;
class QString;
class COverlayText;
class COverlayTextBox;
class COverlayDistance;
class QMenu;

class COverlayDB : public IDB
{
    Q_OBJECT;
    public:

        virtual ~COverlayDB();

        static COverlayDB& self(){return *m_self;}

        void draw(QPainter& p, const QRect& r);

        void loadGPX(CGpx& gpx);
        void saveGPX(CGpx& gpx);
        void loadQLB(CQlb& qlb);
        void saveQLB(CQlb& qlb);
        void upload(){};
        void download(){};
        void clear();

        /// get iterator access to track point list
        QMap<QString,IOverlay*> ::iterator begin(){return overlays.begin();}
        /// get iterator access to track point list
        QMap<QString,IOverlay*> ::iterator end(){return overlays.end();}

        /// delete several overlays by their keys
        void delOverlays(const QStringList& keys);

        COverlayText * addText(const QString& text, const QRect& rect);
        COverlayTextBox * addTextBox(const QString& text, double lon, double lat, const QPoint& anchor, const QRect& rect);
        COverlayDistance * addDistance(const QList<XY>& pts);

        void customMenu(const QString& key, QMenu& menu);

    private:
        friend class CMainWindow;
        friend class COverlayToolWidget;
        COverlayDB(QTabWidget * tb, QObject * parent);

        static COverlayDB * m_self;

        QMap<QString,IOverlay*> overlays;
};

#endif //COVERLAYDB_H

