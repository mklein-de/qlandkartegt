/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CWptToolWidget.cpp

  Module:

  Description:

  Created:     01/17/2008

  (C) 2008


**********************************************************************************************/

#include "CWptToolWidget.h"

#include <QtGui>

CWptToolWidget::CWptToolWidget(QToolBox * parent)
    : QWidget(parent)
{
    parent->addItem(this,QIcon(":/icons/iconWaypoint16x16"),tr("Waypoints"));
}

CWptToolWidget::~CWptToolWidget()
{

}

