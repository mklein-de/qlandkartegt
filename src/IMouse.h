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

#ifndef IMOUSE_H
#define IMOUSE_H

#include <QCursor>
#include <QObject>
#include <QRect>
#include <QPointer>

#include "CTrack.h"
class QMouseEvent;
class QMenu;
class CCanvas;
class CWpt;
class IOverlay;
class CSearch;

/// Base class to all mouse function objects
/**
    The function of the mouse changes depending on the mega menu selection.
    All mouse events will be forwared to a subclass of IMouse. The subclass
    will define the current function and it keeps track of all runtime
    variables.
*/
class IMouse : public QObject
{
    Q_OBJECT;
    public:
        IMouse(CCanvas * parent);
        virtual ~IMouse();

        /// the mouse move event as defined by QWidget::mouseMoveEvent
        virtual void mouseMoveEvent(QMouseEvent * e) = 0;
        /// the mouse press event as defined by QWidget::mousePressEvent
        virtual void mousePressEvent(QMouseEvent * e) = 0;
        /// the mouse release event as defined by QWidget::mouseReleaseEvent
        virtual void mouseReleaseEvent(QMouseEvent * e) = 0;

        /// the current mouse cursor
        /**
            Each mouse function is represented by a special cursor. The main
            widget uses this method to query the current cursor.
        */
        operator const QCursor&(){return cursor;}

        /// draw mouse function spezific elements
        /**
            Some actions demand additional graphical elements to represent the
            state of the acquired runtime variables. E.g. capture rectangle.
            This is the place to draw them.
        */
        virtual void draw(QPainter& ){}

        /// append a context menu by own actions
        virtual void contextMenu(QMenu& ){};

        /// called by CCanvas right befor a new mouse handler is selected.
        virtual void looseFocus(){};

#ifdef GPX_EXTENSIONS
        CGpxExtTr tr_ext;        //TODO: CGpxExtPt -> tr_ext
#endif

    protected:
        /// for internal use to start a semi-transparent capture rectangle
        void startRect(const QPoint& p);
        /// for internal use to set the bottom right of the capture rectangle
        void resizeRect(const QPoint& p);
        /// actually draw the current capture rectangle
        void drawRect(QPainter& p);
        /// draw selected waypoint
        void drawSelWpt(QPainter& p);
        /// draw selected track point
        void drawSelTrkPt(QPainter& p);
        /// draw selected search
        void drawSelSearch(QPainter& p);

        /// choose waypoint close to cursor
        void mouseMoveEventWpt(QMouseEvent * e);
        /// choose track point close to cursor
        void mouseMoveEventTrack(QMouseEvent * e);
        /// choose overlay under cursor
        void mouseMoveEventOverlay(QMouseEvent * e);
        /// choose waypoint close to cursor
        void mouseMoveEventSearch(QMouseEvent * e);

        /// trigger waypoint function
        void mousePressEventWpt(QMouseEvent * e);
        /// trigger search function
        void mousePressEventSearch(QMouseEvent * e);

        /// the functions mouse icon
        QCursor cursor;
        /// pointer to the parent canvas
        CCanvas * canvas;
        /// capture rectangle
        QRect rect;

        /// current selected waypoint
        QPointer<CWpt> selWpt;
        /// current selected trackpoint
        CTrack::pt_t * selTrkPt;
        /// current selected overlay
        static QPointer<IOverlay> selOverlay;

        QRect rectDelWpt;
        QRect rectMoveWpt;
        QRect rectEditWpt;
        QRect rectCopyWpt;
        QRect rectViewWpt;

        QRect rectDelSearch;
        QRect rectConvertSearch;
        QRect rectCopySearch;

        bool doSpecialCursorWpt;
        bool doSpecialCursorSearch;

        QPointer<CSearch> selSearch;
#ifdef GPX_EXTENSIONS
        QPointer<CTrack> track;  //TODO: noch ne def

        int num_of_ext;          //TODO: Anzahl der extensions
                                 //TODO: Namen der extensions
        QList<QString> names_of_ext;
#endif
};
#endif                           //IMOUSE_H
