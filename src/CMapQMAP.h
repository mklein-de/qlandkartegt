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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#ifndef CMAPQMAP_H
#define CMAPQMAP_H

#include "IMap.h"

#include <QVector>
#include <QPointer>
#include <QThread>
#include <QMutex>
#include <QProgressDialog>
#include <QDir>
#include <QProcess>

class CMapDEM;
class CMapLevel;
class CExportMapThread;
class CCanvas;

/// Render object for a GeoTiff raster map set
/**
    The life of this object is tied to the lifetime of the map definition.
    If you want to load a new mapdefinition you have to create a new CMapQMAP
    instance.

*/
class CMapQMAP : public IMap
{
    Q_OBJECT;
    public:
        /**
            @param filename full qualified path to a raster map definition
            @param parent   parent object for the usual Qt stuff
        */
        CMapQMAP(const QString& key, const QString& filename, CCanvas * parent);
        virtual ~CMapQMAP();

        void draw(QPainter& p);
        void draw();
        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);
        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        qint32 getZoomLevel();
        void zoom(qint32& level);
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);
        void select(IMapSelection& ms, const QRect& rect);

    private:
        friend class CExportMapThread;
        void getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale);
        void __test();

        QString exportPath;

        QVector<CMapLevel*> maplevels;

        QPointer<CMapLevel> pMaplevel;

        float zoomFactor;

        /// top left corner as long / lat [rad]
        XY topLeft;
        /// top bottom right as long / lat [rad]
        XY bottomRight;

        bool foundMap;

        bool quadraticZoom;
};
#endif                           //CMAPQMAP_H
