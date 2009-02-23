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

#ifndef CMAPOSM_H
#define CMAPOSM_H

#include <IMap.h>


class CCanvas;

class CMapOSM : public IMap
{
    Q_OBJECT;
    public:
        CMapOSM(CCanvas * parent);
        virtual ~CMapOSM();

        void convertPt2M(double&, double&);
        void convertM2Pt(double&, double&);
        void move(const QPoint&, const QPoint&);
        void zoom(bool, const QPoint&);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void zoom(qint32& level);
        void select(const QRect&){};
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2){lon1 = lon2 = lat1 = lat2 = 0;};

        void draw(QPainter& p);
    private:
        void draw();

        XY topLeft;
        double xscale;
        double yscale;
        double x;
        double y;
        double zoomFactor;
};

#endif //CMAPOSM_H

