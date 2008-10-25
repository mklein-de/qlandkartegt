/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CGarminPoint.cpp

  Module:

  Description:

  Created:     10/22/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CGarminPoint.h"
#include "Garmin.h"
#include "Platform.h"

#include <QtCore>

CGarminPoint::CGarminPoint()
: type(0)
, isLbl6(false)
, hasSubType(false)
, lbl_ptr(0xFFFFFFFF)
{

}

CGarminPoint::~CGarminPoint()
{

}

quint32 CGarminPoint::decode(qint32 iCenterLon, qint32 iCenterLat, quint32 shift, const quint8 * pData)
{
    qint16 dLng, dLat;

    type        = (quint16)(*pData) << 8;

    ++pData;

    lbl_ptr     = gar_ptr_load(uint24_t, pData);
    hasSubType  = lbl_ptr & 0x00800000;
    isLbl6      = lbl_ptr & 0x00400000;
    lbl_ptr     = lbl_ptr & 0x003FFFFF;

    pData += 3;

    dLng = gar_ptr_load(int16_t, pData); pData += 2;
    dLat = gar_ptr_load(int16_t, pData); pData += 2;

    qint32 x1,y1;

    x1 = ((qint32)dLng << shift) + iCenterLon;
    y1 = ((qint32)dLat << shift) + iCenterLat;
    lon = GARMIN_RAD(x1);
    lat = GARMIN_RAD(y1);

#ifdef DEBUG_SHOW_POINTS
    qDebug() << x1 << y1 << point.u << point.v;
#endif

    if(hasSubType) {
        type |= *pData;
        return 9;
    }

    return 8;
}
