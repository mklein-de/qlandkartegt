/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CUnitMetric.cpp

  Module:

  Description:

  Created:     07/08/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CUnitMetric.h"

CUnitMetric::CUnitMetric(QObject * parent)
: IUnit(parent)
{

}

CUnitMetric::~CUnitMetric()
{

}

void CUnitMetric::meter2elevation(float meter, QString& val, QString& unit)
{
    val.sprintf("%1.0f", meter);
    unit = "m";
}

void CUnitMetric::meter2distance(float meter, QString& val, QString& unit)
{
    if(meter < 10){
        val.sprintf("%1.1f", meter);
        unit = "m";
    }
    else if(meter < 1000){
        val.sprintf("%1.0f", meter);
        unit = "m";
    }
    else if(meter < 10000){
        val.sprintf("%1.2f", meter / 1000);
        unit = "km";
    }
    else if(meter < 20000){
        val.sprintf("%1.1f", meter / 1000);
        unit = "km";
    }
    else{
        val.sprintf("%1.0f", meter / 1000);
        unit = "km";
    }
}

