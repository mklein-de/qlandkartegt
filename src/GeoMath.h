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
#ifndef GEOMATH_H
#define GEOMATH_H
//#include <stdint.h>
#include <projects.h>
#include <QVector>


extern void GPS_Math_Deg_To_DegMin(float v, qint32 *d, float *m);
extern void GPS_Math_DegMin_To_Deg(const qint32 d, const float m, float& deg);
extern bool GPS_Math_Str_To_Deg(const QString& str, float& lon, float& lat);
extern void GPS_Math_Deg_To_Str(const float& lon, const float& lat, QString& str);

extern bool testPolygonsForIntersect(const QVector<XY>& poly1, const QVector<XY>& poly2);
extern double distance(const XY& p1, const XY& p2, double& a1, double& a2);

#endif //GEOMATH_H

