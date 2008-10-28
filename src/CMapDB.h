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
#include "CMapSelection.h"
#include <QList>
#include <QMap>
#include <QPointer>

class IMap;
class QPainter;
class CCanvas;
class CMapNoMap;
class CMapEditWidget;
class CMapSearchWidget;
#ifdef PLOT_3D
class CMap3DWidget;
#endif

class CMapDB : public IDB
{
    Q_OBJECT;
    public:
        virtual ~CMapDB();

        static CMapDB& self(){return *m_self;}

        /// open a map collection from disc
        void openMap(const QString& filename, bool asRaster ,CCanvas& canvas);
        /// open a known map by it's key
        void openMap(const QString& key);
        /// close the current map
        void closeMap();

        /// get current main map
        IMap& getMap();

        /// get current DEM map
        IMap& getDEM();

        /// delete known maps by keys
        void delKnownMap(const QStringList& keys);
        /// delete selected maps by keys
        void delSelectedMap(const QStringList& keys);

        void selSelectedMap(const QString& key);

        /// draw visible maps
        void draw(QPainter& p, const QRect& rect);

        void loadGPX(CGpx& gpx);
        void saveGPX(CGpx& gpx);

        void loadQLB(CQlb& qlb);
        void saveQLB(CQlb& qlb);

        void upload();
        void download();

        /// remove all selected map areas
        void clear();
        /// create map edit dialog
        void editMap();
#ifdef PLOT_3D
        /// create tab with 3D map
        void show3DMap(bool show);
#endif
        /// create map search dialog
        void searchMap();

        /// select an area of the map for export [px]
        /**
            @param rect area within the current viewport
        */
        void select(const QRect& rect);

    private:
        friend class CMainWindow;
        friend class CMapToolWidget;
        friend class CMapQMAPExport;

        enum maptype_e {eRaster, eVector};

        struct map_t
        {
            QString     filename;
            QString     description;
            QString     key;
            maptype_e   type;
        };

        CMapDB(QTabWidget * tb, QObject * parent);

        /// get access to known map dictionary, CMapToolWidget only
        const QMap<QString,map_t>& getKnownMaps(){return knownMaps;}
        /// get access to selected map list, CMapToolWidget only
        const QMap<QString,CMapSelection>& getSelectedMaps(){return selectedMaps;}

        void closeVisibleMaps();

        static CMapDB * m_self;

        /// a dictionary of previous opened maps
        QMap<QString,map_t> knownMaps;

        /// the default map if no map is selected
        QPointer<IMap> defaultMap;
        /// the base map
        /**
            The base map will supply the projection.
            All other layers have to use the same projection;
        */
        QPointer<IMap> theMap;

        QPointer<IMap> demMap;

        //         QPointer<IMap> vctMap;

        QPointer<CMapEditWidget> mapedit;
#ifdef PLOT_3D
        QPointer<CMap3DWidget> map3DWidget;
#endif
        QPointer<CMapSearchWidget> mapsearch;

        QMap<QString,CMapSelection> selectedMaps;
};
#endif                           //CMAPDB_H
