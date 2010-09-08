/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        IItem.cpp

  Module:

  Description:

  Created:     09/04/2010

  (C) 2010 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "IItem.h"

#include <QtGui>

quint32 IItem::keycnt = 0;

IItem::IItem(QObject * parent)
: QObject(parent)
, timestamp(QDateTime::currentDateTime().toUTC().toTime_t ())
{

}

IItem::~IItem()
{

}


QString IItem::getKey()
{
    if(key.isEmpty())
    {
        genKey();
    }

    return key;
}

void IItem::genKey()
{
    key = QString("%1%2%3").arg(timestamp).arg(name).arg(keycnt++);
}
