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

#ifndef CCANVAS_H
#define CCANVAS_H

#include "IMap.h"

#include <QWidget>
#include <QPointer>


class IMouse;
class CMouseMoveMap;
class CMouseSelMap;
class CMouseAddWpt;
class CWpt;
class QLabel;
class QSize;
class QPrinter;

/// the map canvas area
class CCanvas : public QWidget
{
    Q_OBJECT
    public:

        enum move_direction_e {eMoveLeft, eMoveRight, eMoveUp, eMoveDown, eMoveCenter};

        CCanvas(QWidget * parent);
        virtual ~CCanvas();

        enum mouse_mode_e {
              eMouseZoomArea    ///< use mouse to define a zoom area
            , eMouseMoveArea    ///< use mouse to move the map
            , eMouseSelectArea  ///< use mouse to select map tiles
            , eMouseAddWpt      ///< use mouse to add waypoints
            //, eMouseEditWpt     ///< use mouse to select waypoints
            //, eMouseMoveWpt     ///< use mouse to drag-n-drop waypoints
            //, eMouseSearchOC    ///< use mouse to define a search radius for open caching
            //, eMouseCutTrack    ///< use mouse to cut a track into two pieces
            //, eMouseEditRte     ///< use mouse to define a new route polyline
            //, eMouseMoveRte     ///< use mouse to move route points
            //, eMouseDelRte      ///< use mouse to delete route points
        };

        /// zoom in/out with a given point as static
        void zoom(bool in, const QPoint& p);

        /// scroll map into given direction
        void move(move_direction_e dir);

        /// center to lon/lat coordinate
        /**
            @param lon the logitude in  [°]
            @param lat the logitude in  [°]
        */
        void move(double lon, double lat);

        void print(QPrinter& printer);

        /// change the current mouse mode
        void setMouseMode(mouse_mode_e mode);

    signals:
        void sigResize(const QSize& size);

    protected:
        void paintEvent(QPaintEvent * e);
        void resizeEvent(QResizeEvent * e);
        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);
        void wheelEvent(QWheelEvent * e);

        /// set override cursor
        void enterEvent(QEvent * event);
        /// restore cursor
        void leaveEvent(QEvent * event);

        void draw(QPainter& p);
        void drawSearchResults(QPainter& p);
        void drawWaypoints(QPainter& p);
        void drawTracks(QPainter& p);


    private:
        friend class CMouseMoveMap;
        friend class CMouseSelMap;
        friend class CMouseAddWpt;
        friend class CStatusCanvas;

        void drawText(const QString& str, QPainter& p, const QPoint& center);
        void mouseMoveEventCoord(QMouseEvent * e);
        void mouseMoveEventWpt(QMouseEvent * e);

        IMouse * mouse;
        CMouseMoveMap * mouseMoveMap;
        CMouseSelMap * mouseSelMap;
        CMouseAddWpt * mouseAddWpt;

        /// current mouse mode
        mouse_mode_e mouseMode;

        /// current mouse position
        QPoint posMouse;

        QPointer<CWpt> selWpt;

        QLabel * info;
};

#endif //CCANVAS_H

