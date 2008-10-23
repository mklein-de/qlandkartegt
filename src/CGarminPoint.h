/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CGarminPoint.h

  Module:

  Description:

  Created:     10/22/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CGARMINPOINT_H
#define CGARMINPOINT_H

#include <QtGlobal>
#include <QStringList>

class CGarminTile;

class CGarminPoint
{
    public:
        CGarminPoint();
        virtual ~CGarminPoint();

        quint32 decode(qint32 iCenterLon, qint32 iCenterLat, quint32 shift, const quint8 * pData);

        quint16 type;
        bool isLbl6;
        bool hasSubType;

        //QString label;
        double lon;
        double lat;

        QStringList labels;
};

#endif //CGARMINPOINT_H

