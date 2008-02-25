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

#ifndef IMOUSE_H
#define IMOUSE_H

#include <QCursor>
#include <QObject>
#include <QRect>
#include <QPointer>
class QMouseEvent;
class CCanvas;
class CWpt;

/// Base class to all mouse function objects
/**
    The function of the mouse changes depending on the mega menu selection.
    All mouse events will be forwared to a subclass of IMouse. The subclass
    will define the current function and it keeps track of all runtime
    variables.
*/
class IMouse : public QObject
{
    Q_OBJECT
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

    protected:
        /// for internal use to start a semi-transparent capture rectangle
        void startRect(const QPoint& p);
        /// for internal use to set the bottom right of the capture rectangle
        void resizeRect(const QPoint& p);
        /// actually draw the current capture rectangle
        void drawRect(QPainter& p);
        /// draw selected waypoint
        void drawSelWpt(QPainter& p);

        /// choose waypoint close to cursor
        void mouseMoveEventWpt(QMouseEvent * e);

        /// the functions mouse icon
        QCursor cursor;
        /// pointer to the parent canvas
        CCanvas * canvas;
        /// capture rectangle
        QRect rect;

        QPointer<CWpt> selWpt;

};

#endif //IMOUSE_H

