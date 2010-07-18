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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#ifndef COVERLAYDISTANCE_H
#define COVERLAYDISTANCE_H

#include "IOverlay.h"
#include "GeoMath.h"

#include <projects.h>
#ifdef __MINGW32__
#undef LP
#endif

class COverlayDistanceEditWidget;


class COverlayDistance : public IOverlay
{
    Q_OBJECT;
    public:

        COverlayDistance(const QString& name, const QString& comment, double speed, const QList<xy>& pts, QObject * parent);
        virtual ~COverlayDistance();

        /// returns true while moving a waypoint
        bool mouseActionInProgress(){return doMove;}
        /// returns name, comment and length
        QString getInfo();
        /// returns true if pt is close as 30px to a waypoint
        bool isCloseEnough(const QPoint& pt);

        /// draw the ployline, waypoints and action icons
        void draw(QPainter& p);

        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);

        /// get last point of polyline
        XY getLast(){return points.last();}

        /// add "Make Track" and "Edit..." to custom menu
        void customMenu(QMenu& menu);

        void save(QDataStream& s);
        void load(QDataStream& s);

        /// iterate over all waypoints to get zoom area
        void makeVisible();

        void looseFocus();

        QRectF getBoundingRectF();

        void delPointsByIdx(const QList<int>& idx);

        static bool isEditMode();

    signals:
        void sigSelectionChanged();

    private slots:
        void slotToTrack();
        void slotToRoute();
        void slotEdit();

    private:
        friend class COverlayDB;
        friend class CDlgEditDistance;
        friend class COverlayDistanceEditWidget;
        void calcDistance();

        /// the polyline as list of points [rad]
        QList<xy> points;
        /// indices of selected points
        QList<int> selectedPoints;
        /// pointer to point of polyline if cursor is closer than 30px
        xy * thePoint;
        /// need to restore point if move command is aborted
        xy savePoint;

        QString name;
        QString comment;
        double distance;
        double speed;

        QRect rectDel;
        QRect rectMove;
        QRect rectAdd1;
        QRect rectAdd2;

        bool doSpecialCursor;
        bool doMove;
        bool doFuncWheel;

        enum addType_e{eNone, eBefore, eAfter, eAtEnd};
        addType_e addType;

        double anglePrev;
        double angleNext;

        QPolygon leadline;

        QPolygon subline;

        static QPointer<COverlayDistanceEditWidget> editwidget;


};

#define OVL_NOFLOAT 1e25f

#endif                           //COVERLAYDISTANCE_H
