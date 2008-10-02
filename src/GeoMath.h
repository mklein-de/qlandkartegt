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

// extern void GPS_Math_Deg_To_DegMin(float v, qint32 *d, float *m);
// extern void GPS_Math_DegMin_To_Deg(const qint32 d, const float m, float& deg);

/// convert a string to a longitude / latitude pair
/**
    The string has to match the format [N|S] ddd mm.sss [W|E] ddd mm.sss

    On error and not silent a messagebox is raised.

    @param str input string
    @param lon reference to store the output longitude []
    @param lat reference to store the output latitude []

*/
extern bool GPS_Math_Str_To_Deg(const QString& str, float& lon, float& lat, bool silent = false);
/// convert a string to a longitude / latitude pair
/**
    The format of the string can be one of:

    * longitude / latitude in degrees: [N|S] ddd mm.sss [W|E] ddd mm.sss
    * easting / northing in meters / feet: EEEEEE.EEEEEE NNNNNN.NNNNNN
    * longitude / latitude in degrees: DDD.ddddd DD.dddddd

    If a projection string is supplied the coordinate is transformed to the given projection.

    On error a messagebox is raised.

    @param str input string
    @param lon reference to store the output longitude / easting []|[m]|[ft]
    @param lat reference to store the output latitude / northing []|[m]|[ft]
    @param projstr a valid proj4 projection string

    @return Return true on success.
*/
extern bool GPS_Math_Str_To_LongLat(const QString& str, float& lon, float& lat, const QString& projstr);
/// convert a longitude / latitude pair to a human readable string
/**
    The output format will be [N|S] ddd mm.sss [W|E] ddd mm.sss

    @param lon the input longitude in []
    @param lon the input latitude in []
    @param str reference to string to store resulting output
*/
extern void GPS_Math_Deg_To_Str(const float& lon, const float& lat, QString& str);

extern bool testPolygonsForIntersect(const QVector<XY>& poly1, const QVector<XY>& poly2);

/// calculate the distance between two WGS84 points
/**
    @param pt1 reference to first input point [rad]
    @param pt2 reference to second input point [rad]
    @param a1 the resulting forward azimuth
    @param a1 the resulting backward azimuth

    @return Return the distance between pt1 and pt2.
*/
extern double distance(const XY& p1, const XY& p2, double& a1, double& a2);

/**
    @param pt1 starting point longlat [rad]
    @param distance the distance to go in [m]
    @param bearing the bearing in [rad]

    @return The function will return the resulting point in [rad]
*/
extern XY GPS_Math_Wpt_Projection(XY& pt1, double distance, double bearing);
#endif                           //GEOMATH_H
