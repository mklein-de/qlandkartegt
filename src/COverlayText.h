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

#ifndef COVERLAYTEXT_H
#define COVERLAYTEXT_H

#include "IOverlay.h"
#include <QRect>

class QTextDocument;

class COverlayText : public IOverlay
{
    Q_OBJECT;
    public:
        COverlayText(const QString& text, const QRect& rect, QObject * parent);
        virtual ~COverlayText();

        bool isCloseEnough(const QPoint& pt) override;
        void draw(QPainter& p, const QRect& viewport) override;

        void mouseMoveEvent(QMouseEvent * e) override;
        void mousePressEvent(QMouseEvent * e) override;
        void mouseReleaseEvent(QMouseEvent * e) override;

        void save(QDataStream& s) override;
        void load(QDataStream& s) override;

        bool mouseActionInProgress() override {return doMove || doSize;}

        QString getInfo() const override;

        QString getName() const override {return getInfo();}

    private:
        friend class COverlayDB;
        QRect rect;
        QRect rectMove;
        QRect rectSize;
        QRect rectEdit;
        QRect rectDel;

        QRect rectDoc;

        QRect rectMouse;

        bool doMove;
        bool doSize;

        bool doSpecialCursor;
        QTextDocument * doc;
};
#endif                           //COVERLAYTEXT_H
