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
#ifndef CMOUSEMOVEWPT_H
#define CMOUSEMOVEWPT_H

#include "IMouse.h"
#include <QPoint>

class CMouseMoveWpt : public IMouse
{
    Q_OBJECT;
    public:
        CMouseMoveWpt(CCanvas * canvas);
        virtual ~CMouseMoveWpt();

        void draw(QPainter& p);
        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);

    private:
        bool moveWpt;
        QPoint newPos;
};

#endif //CMOUSEMOVEWPT_H

