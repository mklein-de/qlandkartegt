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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "GeoMath.h"
#include <stdlib.h>
#include <QtGui>
#include <limits>

#if WIN32
#include <math.h>
#include <float.h>
#ifndef __MINGW32__
typedef __int32 int32_t;
#endif
#define isnan _isnan
#define FP_NAN NAN
#endif

const double WGS84_a = 6378137.0;
const double WGS84_b = 6356752.314245;
const double WGS84_f = 1.0/298.257223563;

bool GPS_Math_Deg_To_DegMin(float v, int32_t *d, float *m)
{
    bool sign = v < 0;
    int32_t deg = abs(v);
    double  min = (fabs(v) - deg) * 60.0;

    *d = deg;
    *m = min;

    return sign;
}


void GPS_Math_DegMin_To_Deg(bool sign, const int32_t d, const float m, float& deg)
{

    deg = abs(d) + m / 60.0;
    if(sign)
    {
        deg = -deg;
    }

    return;
}


bool GPS_Math_Str_To_Deg(const QString& str, float& lon, float& lat, bool silent)
{
    QRegExp re1("^\\s*([N|S]){1}\\W*([0-9]+)\\W*([0-9]+\\.[0-9]+)\\s+([E|W]){1}\\W*([0-9]+)\\W*([0-9]+\\.[0-9]+)\\s*$");

    QRegExp re2("^\\s*([N|S]){1}\\s*([0-9]+\\.[0-9]+)\\W*\\s+([E|W]){1}\\s*([0-9]+\\.[0-9]+)\\W*\\s*$");

    QRegExp re3("^\\s*([-0-9]+\\.[0-9]+)\\s+([-0-9]+\\.[0-9]+)\\s*$");

    if(re2.exactMatch(str))
    {
        bool signLat    = re2.cap(1) == "S";
        float absLat    = re2.cap(2).toDouble();
        lat = signLat ? -absLat : absLat;

        bool signLon    = re2.cap(3) == "W";
        float absLon    = re2.cap(4).toDouble();
        lon = signLon ? -absLon : absLon;
    }
    else if(re1.exactMatch(str))
    {

        bool signLat    = re1.cap(1) == "S";
        int degLat      = re1.cap(2).toInt();
        float minLat    = re1.cap(3).toDouble();

        GPS_Math_DegMin_To_Deg(signLat, degLat, minLat, lat);

        bool signLon    = re1.cap(4) == "W";
        int degLon      = re1.cap(5).toInt();
        float minLon    = re1.cap(6).toDouble();

        GPS_Math_DegMin_To_Deg(signLon, degLon, minLon, lon);
    }
    else if(re3.exactMatch(str))
    {
        lat             = re3.cap(1).toDouble();
        lon             = re3.cap(2).toDouble();
    }
    else
    {
        if(!silent) QMessageBox::warning(0,QObject::tr("Error"),QObject::tr("Bad position format. Must be: \"[N|S] ddd mm.sss [W|E] ddd mm.sss\" or \"[N|S] ddd.ddd [W|E] ddd.ddd\""),QMessageBox::Ok,QMessageBox::NoButton);
        return false;
    }

    if(fabs(lon) > 180.0 || fabs(lat) > 90.0)
    {
        if(!silent) QMessageBox::warning(0,QObject::tr("Error"),QObject::tr("Position values out of bounds. "),QMessageBox::Ok,QMessageBox::NoButton);
        return false;
    }

    return true;
}


/// calc if 2 line segments intersect
/**
    The 1st line is defined by (x11,y11) - (x12,y12)
    The 2nd line is defined by (x21,y21) - (x22,y22)

*/
bool testLineSegForIntersect(float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22)
{
    /*
            float denom = ((other_line.end_.y_ - other_line.begin_.y_)*(end_.x_ - begin_.x_)) -
                          ((other_line.end_.x_ - other_line.begin_.x_)*(end_.y_ - begin_.y_));

            float nume_a = ((other_line.end_.x_ - other_line.begin_.x_)*(begin_.y_ - other_line.begin_.y_)) -
                           ((other_line.end_.y_ - other_line.begin_.y_)*(begin_.x_ - other_line.begin_.x_));

            float nume_b = ((end_.x_ - begin_.x_)*(begin_.y_ - other_line.begin_.y_)) -
                           ((end_.y_ - begin_.y_)*(begin_.x_ - other_line.begin_.x_));
    */
    float denom  = ((y22 - y21) * (x12 - x11)) - ((x22 - x21) * (y12 - y11));
    float nume_a = ((x22 - x21) * (y11 - y21)) - ((y22 - y21) * (x11 - x21));
    float nume_b = ((x12 - x11) * (y11 - y21)) - ((y12 - y11) * (x11 - x21));

    if(denom == 0.0f) return false;

    float ua = nume_a / denom;
    float ub = nume_b / denom;

    if(ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f)
    {
        return true;
    }
    return false;
}


bool testPointInPolygon(const XY& pt, const QVector<XY>& poly1)
{

    bool    c = false;
    int     npol;
    int     i = 0, j = 0;
    XY      p1, p2;              // the two points of the polyline close to pt
    float  x = pt.u;
    float  y = pt.v;

    npol = poly1.count();
    if(npol > 2)
    {

        // see http://local.wasp.uwa.edu.au/~pbourke/geometry/insidepoly/
        for (i = 0, j = npol-1; i < npol; j = i++)
        {
            p1 = poly1[j];
            p2 = poly1[i];

            if ((((p2.v <= y) && (y < p1.v))  || ((p1.v <= y) && (y < p2.v))) &&
                (x < (p1.u - p2.u) * (y - p2.v) / (p1.v - p2.v) + p2.u))
            {
                c = !c;
            }
        }
    }
    return c;
}


bool testPolygonsForIntersect(const QVector<XY>& poly1, const QVector<XY>& poly2)
{

    int n;
    int npol1 = poly1.count();
    int npol2 = poly2.count();

    if(npol1 < 2 || npol2 < 2) return false;

    // test if points of poly1 are within poly2.
    for(n = 0; n < npol1; ++n)
    {
        if(testPointInPolygon(poly1[n],poly2)) return true;
    }

    // test if lines of poly1 intersect with lines from poly2
    int i1 = 0, j1 = 0;
    int i2 = 0, j2 = 0;

    XY  p1, p2, p3, p4;

    for (i1 = 0, j1 = npol1-1; i1 < npol1; j1 = i1++)
    {
        p1 = poly1[j1];
        p2 = poly1[i1];
        for (i2 = 0, j2 = npol2-1; i2 < npol2; j2 = i2++)
        {
            p3 = poly2[j2];
            p4 = poly2[i2];
            if(testLineSegForIntersect(p1.u,p1.v,p2.u,p2.v,p3.u,p3.v,p4.u,p4.v))
            {
                return true;
            }
        }
    }
    return false;
}


// from http://www.movable-type.co.uk/scripts/LatLongVincenty.html
// additional antipodal convergence trick might be a bit lame, but it seems to work
double distance(const XY& p1, const XY& p2, double& a1, double& a2)
{
    double cosSigma = 0.0;
    double sigma = 0.0;
    double sinAlpha = 0.0;
    double cosSqAlpha = 0.0;
    double cos2SigmaM = 0.0;
    double sinSigma = 0.0;
    double sinLambda = 0.0;
    double cosLambda = 0.0;

    double L = p2.u - p1.u;

    double U1 = atan((1-WGS84_f) * tan(p1.v));
    double U2 = atan((1-WGS84_f) * tan(p2.v));
    double sinU1 = sin(U1), cosU1 = cos(U1);
    double sinU2 = sin(U2), cosU2 = cos(U2);
    double lambda = L, lambdaP = (double)(2*PI);
    unsigned iterLimit = 20;

    while (fabs(lambda - lambdaP) > 1e-12)
    {
        if (!iterLimit)
        {
            lambda = PI;
            qDebug() << "No lambda convergence, most likely due to near-antipodal points. Assuming antipodal.";
        }

        sinLambda = sin(lambda);
        cosLambda = cos(lambda);
        sinSigma = sqrt((cosU2*sinLambda) * (cosU2*sinLambda) + (cosU1*sinU2-sinU1*cosU2*cosLambda) * (cosU1*sinU2-sinU1*cosU2*cosLambda));

        if (sinSigma==0)
        {
            return 0;            // co-incident points
        }

        cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
        sigma = atan2(sinSigma, cosSigma);
        sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
        cosSqAlpha = 1 - sinAlpha * sinAlpha;
        cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;

        if (isnan(cos2SigmaM))
        {
            cos2SigmaM = 0;      // equatorial line: cosSqAlpha=0 (6)
        }

        double C = WGS84_f/16 * cosSqAlpha * (4 + WGS84_f * (4 - 3 * cosSqAlpha));
        lambdaP = lambda;

        if (iterLimit--) lambda = L + (1-C) * WGS84_f * sinAlpha * (sigma + C*sinSigma*(cos2SigmaM + C * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
    }

    double uSq = cosSqAlpha * (WGS84_a*WGS84_a - WGS84_b*WGS84_b) / (WGS84_b*WGS84_b);
    double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
    double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));
    double deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
    double s = WGS84_b*A*(sigma-deltaSigma);

    a1 = atan2(cosU2 * sinLambda, cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) * 360 / TWOPI;
    a2 = atan2(cosU1 * sinLambda, -sinU1 * cosU2 + cosU1 * sinU2 * cosLambda) * 360 / TWOPI;
    return s;
}


double parallel_distance(const XY& p1, const XY& p2)
{
    // Assure same latitude V
    if (p1.v != p2.v) return std::numeric_limits<double>::quiet_NaN();

    // Compute the distance between Earth center and latitude V
    double cosV = cos(p1.v);
    double r = WGS84_a*WGS84_b / sqrt(cosV*cosV*WGS84_b*WGS84_b + (1-cosV*cosV)*WGS84_a*WGS84_a);

    // Return the lenght of U2-U1 arc at latitude V
    return fabs(p2.u-p1.u)*r*cosV;
}


void GPS_Math_Deg_To_Str(const float& x, const float& y, QString& str)
{
    qint32 degN,degE;
    float minN,minE;

    bool signLat = GPS_Math_Deg_To_DegMin(y, &degN, &minN);

    bool signLon = GPS_Math_Deg_To_DegMin(x, &degE, &minE);

    QString lat,lng;
    lat = signLat ? "S" : "N";
    lng = signLon ? "W" : "E";
    str.sprintf("%s%02d\260 %06.3f %s%03d\260 %06.3f",lat.toUtf8().data(),abs(degN),minN,lng.toUtf8().data(),abs(degE),minE);
    //print fully metric str.sprintf("%s%06.8f\260 %s%06.8f\260 ",lat.toUtf8().data(),y,lng.toUtf8().data(),x);
}


bool GPS_Math_Str_To_LongLat(const QString& str, float& lon, float& lat, const QString& projstr)
{
    double u = 0, v = 0;
    QRegExp re("^\\s*([\\-0-9\\.]+)\\s+([\\-0-9\\.]+)\\s*$");

    PJ * pjTar = 0;
    if(!projstr.isEmpty())
    {
        pjTar = pj_init_plus(projstr.toLatin1());
        if(pjTar == 0)
        {
            QMessageBox::warning(0,QObject::tr("Error ..."), QObject::tr("Failed to setup projection. Bad syntax?\n%1").arg(projstr), QMessageBox::Abort,QMessageBox::Abort);
            return false;
        }
    }
    PJ * pjWGS84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

    if(GPS_Math_Str_To_Deg(str, lon, lat,true))
    {
        if(pjTar)
        {
            u = lon * DEG_TO_RAD;
            v = lat * DEG_TO_RAD;
            pj_transform(pjWGS84,pjTar,1,0,&u,&v,0);
        }
    }
    else
    {
        if(!re.exactMatch(str))
        {
            QMessageBox::warning(0,QObject::tr("Error ..."), QObject::tr("Failed to read reference coordinate. Bad syntax?\n%1").arg(str), QMessageBox::Abort,QMessageBox::Abort);
            if(pjWGS84) pj_free(pjWGS84);
            if(pjTar) pj_free(pjTar);
            return false;
        }
        u = re.cap(1).toDouble();
        v = re.cap(2).toDouble();

        if((abs(u) <= 180) && (abs(v) <= 90) && pjTar)
        {
            u = u * DEG_TO_RAD;
            v = v * DEG_TO_RAD;
            pj_transform(pjWGS84,pjTar,1,0,&u,&v,0);
        }
    }

    lon = u;
    lat = v;

    if(pjWGS84) pj_free(pjWGS84);
    if(pjTar) pj_free(pjTar);
    return true;
}


XY GPS_Math_Wpt_Projection(XY& pt1, double distance, double bearing)
{
    XY pt2;

    double d    = distance / 6378130.0;
    double lon1 = pt1.u;
    double lat1 = pt1.v;

    double lat2 = asin(sin(lat1) * cos(d) + cos(lat1) * sin(d) * cos(-bearing));
    double lon2 = cos(lat1) == 0 ? lon1 : fmod(lon1 - asin(sin(-bearing) * sin(d) / cos(lat1)) + PI, TWOPI) - PI;

    pt2.u = lon2;
    pt2.v = lat2;
    return pt2;
}
