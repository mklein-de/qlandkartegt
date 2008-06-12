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
#ifndef IOVERLAY_H
#define IOVERLAY_H

#include <QObject>
#include <QPixmap>

class QPainter;
class QMouseEvent;

class IOverlay : public QObject
{
    Q_OBJECT;
    public:
        IOverlay(QObject * parent, const QString& type, const QPixmap& icon);
        virtual ~IOverlay();

        virtual void draw(QPainter& p) = 0;
        virtual QString getInfo(){return tr("No info set");}
        virtual QRect getRect() = 0;

        virtual bool mouseActionInProgress(){return false;}

        virtual void mouseMoveEvent(QMouseEvent * e){};
        virtual void mousePressEvent(QMouseEvent * e){};
        virtual void mouseReleaseEvent(QMouseEvent * e){};


        void select(IOverlay * s){selected = s;}

        const QString type;
        const QPixmap icon;
        const QString key;

    protected:
        static IOverlay * selected;
};

#endif //IOVERLAY_H

