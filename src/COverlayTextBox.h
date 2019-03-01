/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/

#ifndef COVERLAYTEXTBOX_H
#define COVERLAYTEXTBOX_H

#include "IOverlay.h"
#include <QRect>
#include <QPolygon>
class QPointF;
class QTextDocument;

class COverlayTextBox : public IOverlay
{
    Q_OBJECT;
    public:
        COverlayTextBox(const QString& text, double lon, double lat, const QPoint& anchor, const QRect& rect, QObject * parent);
        virtual ~COverlayTextBox();

        void draw(QPainter& p, const QRect& viewport) override;

        static QPolygonF makePolyline(const QPoint& anchor, const QRect& r);

        bool isCloseEnough(const QPoint& pt) override;

        void mouseMoveEvent(QMouseEvent * e) override;
        void mousePressEvent(QMouseEvent * e) override;
        void mouseReleaseEvent(QMouseEvent * e) override;

        void save(QDataStream& s) override;
        void load(QDataStream& s) override;

        bool mouseActionInProgress() override {return doMove || doSize || doPos;}

        QString getInfo() const override;

        QString getName() const override {return getInfo();}

        void makeVisible() override;

        QRectF getBoundingRectF() override;

    private:
        friend class COverlayDB;
        /// anchor point longitude [rad]
        double lon;
        /// anchor point latitude [rad]
        double lat;
        /// the text box rectangle [px]
        QRect rect;
        /// the anchor point [px]
        QPoint pt;
        /// the resulting polylin, normalized to the anchor point
        QPolygonF polyline;

        QRect rectMove;
        QRect rectSize;
        QRect rectEdit;
        QRect rectDel;
        QRect rectAnchor;

        QRect rectDoc;

        QTextDocument * doc;

        bool doMove;
        bool doSize;
        bool doPos;
        bool doSpecialCursor;
};
#endif                           //COVERLAYTEXTBOX_H
