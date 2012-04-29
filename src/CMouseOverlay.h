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

#ifndef CMOUSEOVERLAY_H
#define CMOUSEOVERLAY_H

#include "IMouse.h"

class CMouseOverlay : public IMouse
{
    Q_OBJECT;
    public:
        CMouseOverlay(CCanvas * canvas);
        virtual ~CMouseOverlay();

        void keyPressEvent(QKeyEvent * e);
        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);
};
#endif                           //CMOUSEOVERLAY_H
