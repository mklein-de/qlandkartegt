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

#ifndef IMAP_H
#define IMAP_H

#include <QObject>
#include <QRect>
#include <QSize>
#include <QImage>
#include <QPointer>

#include <projects.h>

class QPainter;
class CCanvas;
class CMapDEM;
class IMapSelection;

/// base class to any map render object
class IMap : public QObject
{
    Q_OBJECT;
    public:
        enum maptype_e {eRaster, eGarmin, eDEM};

        IMap(maptype_e type, const QString& key, CCanvas * parent);
        virtual ~IMap();

        enum overlay_e {eNone, eShading, eContour};

        /// draw map
        virtual void draw(QPainter& p);
        /// just draw map to internal buffer
        virtual void draw();
        /// draw map as overlay
        virtual void draw(const QSize& s, bool needsRedraw, QPainter& p){}
        /// convert a point on the screen [px] to world coordinates [m]
        /**
            The conversion will be done in place.

            @param u x (longitude) value
            @param v y (latitude) value
        */
        virtual void convertPt2M(double& u, double& v) = 0;
        /// convert world coordinates [m] into a point on the screen [px]
        /**
            The conversion will be done in place.

            @param u longitude (x) value
            @param v latitude (y) value
        */
        virtual void convertM2Pt(double& u, double& v) = 0;
        /// convert a point on the screen [px] to geo. coordinates [rad]
        /**
            The conversion will be done in place.

            @param u x (longitude) value
            @param v y (latitude) value
        */
        virtual void convertPt2Rad(double& u, double& v);

        /// convert geo. coordinates [rad] into cartesian coordinates [m]
        /**
            The conversion will be done in place.

            @param u longitude (x) value
            @param v latitude (y) value
        */
        virtual void convertRad2M(double& u, double& v);

        /// convert cartesian coordinates [m] into geo. coordinates [rad]
        /**
            The conversion will be done in place.

            @param u longitude (x) value
            @param v latitude (y) value
        */
        virtual void convertM2Rad(double& u, double& v);

        /// convert geo. coordinates [rad] into a point on the screen [px]
        /**
            The conversion will be done in place.

            @param u longitude (x) value
            @param v latitude (y) value
        */
        virtual void convertRad2Pt(double& u, double& v);
        /// move the map [px]
        /**
            @param old the (old) starting point
            @param next the (new) location of the starting point
        */
        virtual void move(const QPoint& old, const QPoint& next) = 0;
        /// zoom map around point [px]
        /**
            @param zoomIn set true to increase resolution and false to decrease resolution
            @param p      define center (steady point) of the transformation
        */
        virtual void zoom(bool zoomIn, const QPoint& p) = 0;
        /// zoom map to fit area
        /**
            @param lon1 the westbound value in [rad]
            @param lat1 the northbound value in [rad]
            @param lon2 the eastbound value in [rad]
            @param lat2 the southbound value in [rad]
        */
        virtual void zoom(double lon1, double lat1, double lon2, double lat2) = 0;

        /// set map to a certain zoom level
        /**
            level <  1 overzoom
            level == 1 no zoom
            level >  1 zoom out
        */
        virtual void zoom(qint32& level) = 0;

        ///
        virtual qint32 getZoomLevel() { return 1; }

        /// get the top left and bottom right corner
        /**
            @param lon1 reference to store westbound longitude in [rad]
            @param lat1 reference to store northbound latitude in [rad]
            @param lon2 reference to store eastbound longitude in [rad]
            @param lat2 reference to store southbound latitude in [rad]
        */
        virtual void dimensions(double& lon1, double& lat1, double& lon2, double& lat2) = 0;

        /// get the elevation of the given point
        /**

            @param lon the longitude in [rad]
            @param lat the latitude in [rad]

            @return The elevation at the point or WPT_NOFLOAT if no elevation data is loaded.
        */
        virtual float getElevation(double lon, double lat);

        /// get the map's filename
        virtual const QString& getFilename(){return filename;}

        /// get size of viewport in [px]
        virtual const QSize& getSize(){return size;}
        /// return the key for registered maps
        /**
            @return A string for registered maps. Empty string for all others
        */
        const QString& getKey(){return key;}

        /// get read access to the internally used pixmap buffer
        const QImage& getBuffer(){return buffer;}

        /// get proj4 compatible projection string
        char * getProjection();

        /// a DEM overlay has to register itself at the map it overlays
        /**
            The default implementation is too check if both map objects use the
            same projection. However vector maps might use this to adjust their
            internal projection to the DEM's one.
        */
        virtual void registerDEM(CMapDEM& dem);

        /// add a vector map as overlay map
        /**
            This call is passed on to any existing overlay map. Once
            the last overlay is reached it will use the map key to
            create an overlay map with the help of CMapDB.
        */
        virtual void addOverlayMap(const QString& key);

        /// remove overlay map by key
        /**
            This call is passed to ovlMap until the key matches. The
            overlay is removed from the overlay pointer chain.
        */
        virtual void delOverlayMap(const QString& key);

        /// test if map is used as overlay
        virtual bool hasOverlayMap(const QString& key);

        /// get pointer to overlay map
        virtual IMap * getOverlay(){return ovlMap;}

        /// select map area for export or further processing
        virtual void select(IMapSelection& ms, const QRect& rect){};
        /**
         * get values for the defined region.
         * \param buffer - elevation matrix. It must has size w * h.
         * \param topLeft - geo coordinates in [rad] of top left corner
         * \param bottomRight - goe coordinates in [rad] of bottom right corner
         * \param w - matrix width
         * \param h - matrix height
         */
        virtual void getRegion(float *buffer, XY topLeft, XY bottomRight, int width, int height) {};

        bool getNeedsRedraw(){return needsRedraw;}

        const maptype_e maptype;

        virtual bool getFastDrawFlag() { return doFastDraw; };
    signals:
        void sigChanged();
        void sigResize(const QSize& size);

    public slots:
        /// change visible size of map
        /**
            @param size size of the new viewport (display) area [px]
        */
        virtual void resize(const QSize& size);

    protected slots:
        /// called by timerFastDraw to reset doFastDraw flag
        virtual void slotResetFastDraw();

        virtual void slotOvlChanged();

    protected:
        virtual void getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale){};
        virtual void getArea_n_Scaling_fromBase(XY& p1, XY& p2, float& my_xscale, float& my_yscale);
        virtual void setFastDraw();

        QString filename;
        /// canvas / viewport rectangle [px]
        QRect rect;
        /// canvas / viewport size
        QSize size;
        /// absolute zoom factor (1...x)
        qint32 zoomidx;

        /// source projection of the current map file
        /**
            Has to be set by subclass. Destruction has to be
            handeled by subclass.
        */
        PJ * pjsrc;
        /// target projection
        /**
            Is set by IMap() to WGS84. Will be freed by ~IMap()
        */
        PJ * pjtar;
        /// set true if the content on display has changed (zoom, move)
        bool needsRedraw;
        /// the key used to register the map
        QString key;
        /// the internal pixmap buffer to draw a map on
        QImage buffer;

        QPointer<IMap> ovlMap;

        bool doFastDraw;
        QTimer * timerFastDraw;

};
#endif                           //IMAP_H
