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

#ifndef CMAPRASTER_H
#define CMAPRASTER_H

#include "IMap.h"

#include <QVector>
#include <QPointer>
#include <QThread>
#include <QMutex>
#include <QProgressDialog>
#include <QDir>

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
        CMapQMAP(const QString& filename, CCanvas * parent);
        virtual ~CMapQMAP();

        void draw(QPainter& p);
        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);
        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void select(const QRect& rect);
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);
        float getElevation(float lon, float lat);

    private:
        friend class CExportMapThread;

        void zoom(qint32& level);

        QString exportPath;

        QVector<CMapLevel*> maplevels;

        QPointer<CMapLevel> pMaplevel;

        float zoomFactor;

        /// top left corner as long / lat [rad]
        XY topLeft;
        /// top bottom right as long / lat [rad]
        XY bottomRight;

        CExportMapThread * thExportMap;

        QProgressDialog progressExport;

        QPushButton * butCancelExport;

        CMapDEM * pDEM;

};

/// thread object to export a sub-area from a map set
/**

 */
class CExportMapThread : public QThread
{
    Q_OBJECT;
    public:
        CExportMapThread(CMapQMAP * map);

        ~CExportMapThread(){};

        void setup(const XY& p1, const XY& p2, const QString& filename, const QString& comment);

        signals:
        void sigSetMessage(const QString& msg);
        void sigSetRange(int minimum, int maximum);
        void sigSetValue(int progress);
        void sigDone(int r);

    protected:
        void run();

    private slots:
        void slotCancel();

    private:
        QMutex mutex;
        CMapQMAP * theMap;

        QDir exportPath;
        QString filebasename;
        QString comment;
        XY topLeft;
        XY bottomRight;
        double width;
        double height;
        bool canceled;
};
#endif                           //CMAPRASTER_H
