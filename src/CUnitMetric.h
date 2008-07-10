/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CUnitMetric.h

  Module:

  Description:

  Created:     07/08/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CUNITMETRIC_H
#define CUNITMETRIC_H

#include "IUnit.h"

class CUnitMetric : public IUnit
{
    Q_OBJECT;
    public:
        CUnitMetric(QObject * parent);
        virtual ~CUnitMetric();

        void meter2elevation(float meter, QString& val, QString& unit);
        void meter2distance(float meter, QString& val, QString& unit);
};

#endif //CUNITMETRIC_H

