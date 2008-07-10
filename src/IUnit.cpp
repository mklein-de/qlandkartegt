/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        IUnit.cpp

  Module:

  Description:

  Created:     07/08/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "IUnit.h"

IUnit * IUnit::m_self = 0;

IUnit::IUnit(QObject * parent)
: QObject(parent)
{
    //there can be only one...
    if(m_self) delete m_self;
    m_self = this;
}

IUnit::~IUnit()
{

}

