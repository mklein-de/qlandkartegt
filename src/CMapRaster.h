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

class CMapLevel;
class CExportMapThread;

/// Render object for a GeoTiff raster map set
/**
    The life of this object is tied to the lifetime of the map definition.
    If you want to load a new mapdefinition you have to create a new CMapRaster
    instance.

*/
class CMapRaster : virtual public IMap
{
    Q_OBJECT
    public:
        /**
            @param filename full qualified path to a raster map definition
            @param parent   parent object for the usual Qt stuff
        */
        CMapRaster(const QString& filename, QObject * parent);
        virtual ~CMapRaster();

        void draw(QPainter& p);
        void convertPt2M(double& u, double& v);
        void convertM2Pt(double& u, double& v);
        void move(const QPoint& old, const QPoint& next);
        void zoom(bool zoomIn, const QPoint& p);
        void select(const QRect& rect);


    private:
        friend class CExportMapThread;

        void zoom(quint32& level);

        QString filename;

        QVector<CMapLevel*> maplevels;

        QPointer<CMapLevel> pMaplevel;

        quint32 zoomFactor;

        /// top left corner as long / lat [rad]
        XY topLeft;

        CExportMapThread * thExportMap;

        QProgressDialog progressExport;

        QPushButton * butCancelExport;

};

/// thread object to export a sub-area from a map set
/**

*/
class CExportMapThread : public QThread
{
    Q_OBJECT
    public:
        CExportMapThread(CMapRaster * map);

        ~CExportMapThread(){};

        void setup(const XY& p1, const XY& p2);

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

        CMapRaster * theMap;
        QString filename;
        XY topLeft;
        XY bottomRight;
        double width;
        double height;
        bool canceled;
};


#endif //CMAPRASTER_H

