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

#ifndef CMAPNOMAP_H
#define CMAPNOMAP_H

#include "IMap.h"

#include "CCanvas.h"
/// dummy render object
/**
    This is used as place holder if no map is loaded
*/
class CMapNoMap : public IMap
{
    Q_OBJECT
    public:
        CMapNoMap(CCanvas * parent) : IMap(parent){};
        virtual ~CMapNoMap(){};

        void convertPt2M(double&, double&){};
        void convertM2Pt(double&, double&){};
        void move(const QPoint&, const QPoint&){};
        void zoom(bool, const QPoint&){};
        void select(const QRect&){};
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2){lon1 = lon2 = lat1 = lat2 = 0;};
};

#endif //CMAPNOMAP_H

