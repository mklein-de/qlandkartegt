/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CMOUSESELWPT_H
#define CMOUSESELWPT_H

#include "IMouse.h"

class CMouseSelWpt : public IMouse
{
    Q_OBJECT;
    public:
        CMouseSelWpt(CCanvas * canvas);
        virtual ~CMouseSelWpt();

        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);
        void draw(QPainter& p);

    private:
        bool mousePressed;
        QPointF center;
        QPointF point1;

};

#endif //CMOUSESELWPT_H

