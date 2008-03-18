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
#ifndef CMAPDB_H
#define CMAPDB_H

#include "IDB.h"
#include "CMapNoMap.h"
#include <QList>
#include <QMap>

class IMap;
class QPainter;
class CCanvas;
class CMapNoMap;
class CStatusCanvas;
class CMapEditWidget;

class CMapDB : public IDB
{
    Q_OBJECT;
    public:
        virtual ~CMapDB();

        struct map_t
        {
            QString filename;
            QString description;
            QString key;
        };

        static CMapDB& self(){return *m_self;}

        /// open a map collection from disc
        void openMap(const QString& filename, CCanvas& canvas);
        /// open a known map by it's key
        void openMap(const QString& key);
        /// close the current map
        void closeMap();

        /// get access to known map dictionary
        const QMap<QString,map_t>& getKnownMaps(){return knownMaps;}
        /// get current main map
        IMap& getMap() {
            theMap = visibleMaps.isEmpty() ? defaultMap : visibleMaps.at(0);
            return *theMap;
        }
        /// delete known maps by keys
        void delKnownMap(const QStringList& keys);

        /// draw visible maps
        void draw(QPainter& p);

        void loadGPX(CGpx& gpx);
        void saveGPX(CGpx& gpx);

        void loadQLB(CQlb& qlb);
        void saveQLB(CQlb& qlb);

        void upload();
        void download();

        void clear(){};

        void editMap();

    private:
        friend class CMainWindow;

        CMapDB(QTabWidget * tb, QObject * parent);

        void closeVisibleMaps();

        static CMapDB * m_self;

        /// list of all visible maps
        /**
            All maps in this list a drawn from index 0 first to index N.
            The map at index 0 is called main map. All other maps must use the
            same projection as the main map.
        */
        QList<IMap*> visibleMaps;

        /// a dictionary of previous opened maps
        QMap<QString,map_t> knownMaps;

        /// the default map if no map is selected
        CMapNoMap * defaultMap;

        CStatusCanvas * statusCanvas;

        QPointer<IMap> theMap;

        QPointer<CMapEditWidget> mapedit;
};
#endif                           //CMAPDB_H
