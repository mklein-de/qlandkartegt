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

#ifndef COVERLAYDISTANCE_H
#define COVERLAYDISTANCE_H

#include "IOverlay.h"

#include <projects.h>

class COverlayDistance : public IOverlay
{
    Q_OBJECT;
    public:
        COverlayDistance(const QString& name, const QString& comment, const QList<XY>& pts, QObject * parent);
        virtual ~COverlayDistance();

        /// returns true while moving a waypoint
        bool mouseActionInProgress(){return doMove;}
        /// returns name, comment and length
        QString getInfo();
        /// returns true if pt is close as 30px to a waypoint
        bool isCloseEnought(const QPoint& pt);

        /// draw the ployline, waypoints and action icons
        void draw(QPainter& p);

        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);

        /// add a point at the end of the polyline
        void addPoint(XY& pt);
        /// get last point of polyline
        XY getLast(){return points.last();}

        /// add "Make Track" and "Edit..." to custom menu
        void customMenu(QMenu& menu);

        void save(QDataStream& s);
        void load(QDataStream& s);

        /// iterate over all waypoints to get zoom area
        void makeVisible();

    private slots:
        void slotToTrack();
        void slotEdit();

    private:
        friend class COverlayDB;
        friend class CDlgEditDistance;
        void calcDistance();

        /// the polyline as list of points [rad]
        QList<XY> points;
        /// pointer to point of polyline if cursor is closer than 30px
        XY * thePoint;

        QString name;
        QString comment;
        double distance;

        QRect rectDel;
        QRect rectMove;
        QRect rectAdd1;
        QRect rectAdd2;

        bool doSpecialCursor;
        bool doMove;
};
#endif                           //COVERLAYDISTANCE_H
