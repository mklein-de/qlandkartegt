/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CPlotAxisTime.h

  Module:

  Description:

  Created:     04/05/2009

  (C) 2009 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CPLOTAXISTIME_H
#define CPLOTAXISTIME_H

#include "CPlotAxis.h"

class CPlotAxisTime : public CPlotAxis
{
    Q_OBJECT;
    public:
        CPlotAxisTime(QObject * parent);
        virtual ~CPlotAxisTime();

        ///calculate format for the given value
        const QString fmtsgl(double /*val*/){return strFormat;}
        ///calculate format for the given value
        const QString fmtdbl(double /*val*/){return strFormat;}

        const TTic* ticmark( const TTic * t );
    protected:
        void calc();

        QString strFormat;
};

#endif //CPLOTAXISTIME_H

